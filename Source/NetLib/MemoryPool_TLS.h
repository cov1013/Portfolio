#pragma once
#include "MemoryPool.h"

namespace cov1013
{
	template <class T>
	class MemoryPool_TLS
	{
		/// <summary>
		/// 청크 요소
		/// </summary>
		struct ChunkElement
		{
			using Guard = char*;
			static constexpr size_t PaddingSize = 64 - sizeof(Guard);

#ifdef __SAFE_MODE__
			Guard	FrontGuard;
			char	Padding[PaddingSize];
			T		Data;
			Guard	RearGuard;
#else
			T		Data;
#endif
			void*	pChunk = nullptr;
		};

		/// <summary>
		/// 청크 (각 스레드에 해당 청크가 세팅되어, 청크 요소를 할당하고 반환하는 역할을 한다.)
		/// </summary>
		struct Chunk
		{
			long			AllocCount = 0;		// 할당 횟수 (MAX 도달 시 청크를 새롭게 세팅)
			long			FreeCount = 0;		// 반환 횟수 (MAX 도달 시 해당 청크는 메모리풀에 반환)

			// 청크 데이터를 스택으로 선언할 경우, 데이터 생성자에 힙 할당 코드가 있는 경우 해당 Chunk가 생성될 때 힙 할당이 발생한다.
			// 이후 PlacementNew를 사용하여 ChunkElement의 생성자를 호출하면, 힙 할당이 발생한 상태에서 또 다시 힙 할당이 발생하게 된다.
			// 따라서 메모리 누수가 발생한다. 이를 방지하기 위해 ChunkElement를 힙에 할당하여 사용한다.
			ChunkElement*	pElements;			// 실제 할당해줄 메모리 요소 
#ifdef __SAFE_MODE__
			char*			Code = nullptr;		// 해당 메모리풀의 고유 키값
#endif
			int				ElementsCount = 0;	// 청크 요소 개수

			~Chunk()
			{
				for (int i = 0; i < ElementsCount; i++)
				{
					(&(pElements[i].Data))->~T();
				}

				free(pElements);
			}

			bool IsFirstAlloc() const 
			{
				return false == *(bool*)((char*)this + sizeof(Chunk));
			}

			void SetFirstAllocFlag(bool bFlag)
			{
				*(bool*)((char*)this + sizeof(Chunk)) = bFlag;
			}
		};

	public:
		/// <summary>
		/// 생성자
		/// </summary>
		/// <param name="capacity">최초 TLS 메모리풀 용량(해당 메모리풀 사용 시 런타임에 상승할 것. 다만 계속 상승한다면 메모리릭을 의심해봐야한다)</param>
		/// <param name="bPlacementNewFlag">메모리를 할당 받을 때마다 생성자 호출 여부 판단 플래그</param>
		/// <param name="chunkElementCount">청크 메모리 개수</param>
		MemoryPool_TLS(int capacity, bool bPlacementNewFlag, int chunkElementCount)
			: _chunkPool(capacity, false)
			, _chunkElementCount(chunkElementCount)
			, _bPlacementNewFlag(bPlacementNewFlag)
			, _tlsIndex(TlsAlloc())
#ifdef _SAFE_MODE
			// 런타임에 고유한 값을 코드로 사용하기 위해 힙에서 할당 받은 주소를 사용.
			, _code(static_cast<char*>(malloc(sizeof(char))))
#endif
		{
		}

		/// <summary>
		/// 소멸자
		/// </summary>
		~MemoryPool_TLS()
		{
			// TLS 인덱스 반환
			TlsFree(_tlsIndex);

			// 안전 모드에서 사용한 코드 메모리 반환
			if (_code)
			{
				free(_code);
			}
		}

		/// <summary>
		/// 메모리 할당
		/// </summary>
		/// <returns></returns>
		T* Alloc()
		{
			Chunk* pChunk = static_cast<Chunk*>(TlsGetValue(_tlsIndex));

			// TLS에 청크가 없으면 메모리풀에서 청크를 할당받아 TLS에 세팅
			if (pChunk == nullptr)
			{
				pChunk = _chunkPool.Alloc();	// 경합

				// 청크 세팅
				pChunk->AllocCount = 0;
				pChunk->FreeCount = 0;

				// 메모리풀에서 최초 할당인 경우에만 청크 요소를 초기화
				if (pChunk->IsFirstAlloc())
				{
#ifdef __SAFE_MODE__
					pChunk->Code = _code;
#endif
					// 캐시 히트 향상을 위해 청크를 뭉쳐서 힙에 한 번에 할당한다.
					pChunk->pElements = static_cast<ChunkElement*>(malloc(sizeof(ChunkElement) * _chunkElementCount));
					pChunk->ElementsCount = _chunkElementCount;
					for (int i = 0; i < _chunkElementCount; i++)
					{
						// 이후 각 요소 초기화 진행
						ChunkElement* pElement = &pChunk->pElements[i];
#ifdef __SAFE_MODE__
						pElement->FrontGuard = _code;
						pElement->RearGuard = _code;
#endif
						pElement->pChunk = (void*)pChunk;

						// PlacementNewFlag가 false인 경우 메모리 최초 할당 시 생성자 한 번만 호출
						if (_bPlacementNewFlag == false)
						{
							new (&pElement->Data) T();
						}
					}

					// 청크 세팅에 완료했다면, 해당 청크가 반환되어 재할당 되었을 때
					// 해당 청크를 재세팅하지 않게끔 Flag를 true로 변경
					pChunk->SetFirstAllocFlag(true);
				}

				// 할당받은 청크를 TLS에 저장
				TlsSetValue(_tlsIndex, reinterpret_cast<LPVOID>(pChunk));
			}

			T* pData = &(pChunk->pElements[pChunk->AllocCount].Data);

			// PlacementNew 사용시 Element 생성자 호출
			if (_bPlacementNewFlag == true)
			{
				new (pData) T();
			}

			// 청크 요소를 전부 사용했다면 TLS에 NULL을 세팅해서 
			// 다음 할당시 메모리풀에서 새로운 청크를 할당받을 수 있게 한다.
			pChunk->AllocCount = pChunk->AllocCount + 1;
			if (pChunk->AllocCount == _chunkElementCount)
			{
				TlsSetValue(_tlsIndex, nullptr);
			}

			return pData;
		}

		/// <summary>
		/// 메모리 반환
		/// </summary>
		/// <param name="pData"></param>
		/// <returns></returns>
		bool Free(T* pData)
		{
#ifdef __SAFE_MODE__
			pChunkElement = (ChunkElement*)((char*)pData - (sizeof(char*) + PADDING_SIZE));
			pChunk = (Chunk*)(pChunkElement->pChunk);

			if (pChunk->Code != pChunkElement->FrontGuard || pChunk->Code != pChunkElement->RearGuard)
			{
				return false;
			}
#else
			ChunkElement* pChunkElement = reinterpret_cast<ChunkElement*>(pData);
			Chunk* pChunk = static_cast<Chunk*>(pChunkElement->pChunk);
#endif

			// PlacementNew 사용시 반환할 때 마다 소멸자 호출
			if (_bPlacementNewFlag == true)
			{
				(pData)->~T();
			}

			// 반환 횟수 증가
			// 할당 할 때는 할당하려는 스레드만 청크 요소에 접근하지만, 이후 할당된 청크 요소가
			// 다른 스레드에서 접근 할 수 있으므로, 반환 횟수는 원자적으로 접근한다.
			const auto FreeCount = InterlockedIncrement(&pChunk->FreeCount);

			if(FreeCount > _chunkElementCount)
			{
				__debugbreak();
			}

			// 모든 요소를 반환했다면, 청크를 메모리풀에 반환
			if (FreeCount == _chunkElementCount)
			{
				_chunkPool.Free(pChunk);
			}

			return true;
		}

		inline long GetCapacity() const { return _chunkPool.GetCapacity(); }
		inline long GetUseCount() const { return _chunkPool.GetUseCount(); }

	private:
		MemoryPool<Chunk>	_chunkPool;
		int					_chunkElementCount = 0;
		bool				_bPlacementNewFlag = false;
		DWORD				_tlsIndex = TLS_OUT_OF_INDEXES;
		char*				_code = nullptr;
	};
}