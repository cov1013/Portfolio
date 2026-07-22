#pragma once
#pragma warning(disable:4324)
#pragma warning(disable:6011)
#pragma warning(disable:4127)
#include "pch.h"

namespace cov1013
{
	template <typename T>
	class MemoryPool
	{
		struct Node
		{
			using Guard = char*;
			static constexpr size_t PaddingSize = 64 - sizeof(Guard);

#ifdef _SAFE_MODE
			Guard	pFrontGuard;
			char	Padding[PaddingSize];	// Guard와 Data 사이에 Padding을 넣어, Data가 64byte 경계에 정렬되도록 함.
			T		Data;
			bool	bInitializeFlag;
			Guard	pRearGuard;
#else				 
			T		Data;
			bool	bInitializeFlag;
#endif				 
			Node*	pNext;
		};

		struct alignas(16) TopContainer
		{
			Node*				pNode = nullptr;
			unsigned long long	Key = 0;
		};

	public:
		/// <summary>
		/// 생성자
		/// </summary>
		/// <param name="capacity">메모리풀 초기 용량</param>
		/// <param name="bPlacementNewFlag">플레이스먼트 New 사용 여부</param>
		MemoryPool(long capacity, bool bPlacementNewFlag)
		{
			_bPlacementNewFlag = bPlacementNewFlag;
#ifdef _SAFE_MODE
			// 런타임에 고유한 값을 코드로 사용하기 위해 힙에서 할당 받은 주소를 사용.
			_code = static_cast<char*>(malloc(sizeof(char)));
#endif
			for (auto i = 0; i < capacity; i++)
			{
				CreateNode();
			}

			_capacity = capacity;
		}

		/// <summary>
		/// 소멸자
		/// </summary>
		~MemoryPool()
		{
			// 리스트 순회하며 반환
			Node* pDel;
			while (_topContainer.pNode != nullptr)
			{
				pDel = _topContainer.pNode;

				_topContainer.pNode = pDel->pNext;

				// 플레이스먼트 New 미사용 시 힙에 메모리 반환할 때 소멸자 딱 한 번 호출
				if (_bPlacementNewFlag == false)
				{
					(&pDel->Data)->~T();
				}

				// 메모리 사이즈에 따른 메모리 반환 방식 선택
				constexpr auto DataSize = sizeof(T);
				if (DataSize > 64)
				{
					_aligned_free(pDel);
				}
				else
				{
					free(pDel);
				}
				--_capacity;
			}

			// 안전 모드에서 사용한 코드 메모리 반환
			if (_code)
			{
				free(_code);
			}

			// 할당 받은 모든 메모리가 반환되었는지 확인
			if (_capacity != 0)
			{
				__debugbreak();
			}
		}

		/// <summary>
		/// 메모리 할당
		/// </summary>
		/// <returns>할당된 메모리의 포인터</returns>
		T* Alloc()
		{
			// 노드 부족하면 추가 생성
			const long Capacity = _capacity;
			const long UsedCount = _InterlockedIncrement(&_useCount);
			if (Capacity < UsedCount)
			{
				CreateNode();
				_InterlockedIncrement(&_capacity);
			}
			
			// 노드 꺼내기
			TopContainer topContainer = {};
			while (true)
			{
				topContainer.Key = _topContainer.Key;
				topContainer.pNode = _topContainer.pNode;

				const auto _Destination = reinterpret_cast<LONG64 volatile*>(&_topContainer);
				const auto _ExchangeHigh = topContainer.Key + 1;
				const auto _ExchangeLow = reinterpret_cast<LONG64>(topContainer.pNode->pNext);
				const auto _ComparandResult = reinterpret_cast<LONG64*>(&topContainer);
				const bool isSuccess = _InterlockedCompareExchange128(_Destination, _ExchangeHigh, _ExchangeLow, _ComparandResult);

				// 변경 성공했으면 탈출
				if (isSuccess)
				{
					break;
				}

				_mm_pause();
			}

			T* pData = &(topContainer.pNode->Data);

			// 플레이스먼트 New 옵션이 켜져있다면, 생성자 호출.
			if (_bPlacementNewFlag)
			{
				new (pData) T();
			}

			return pData;
		}

		/// <summary>
		/// 메모리 반환
		/// </summary>
		/// <param name="pData">반환할 메모리 포인터</param>
		/// <returns>반환 결과</returns>
		bool Free(T* pData)
		{
#ifdef _SAFE_MODE
			char* pDataChar = reinterpret_cast<char*>(pData);
			const auto alignSize = sizeof(Node::Guard) + Node::PaddingSize;
			Node* pReturn = reinterpret_cast<Node*>(pDataChar - alignSize);

			// 반환 노드가 해당 메모리풀에서 할당한 메모리가 맞는지 확인
			if (_code != pReturn->pFrontGuard || _code != pReturn->pRearGuard)
			{
				return false;
			}
#else
			Node* pReturn = reinterpret_cast<Node*>(pData);
#endif
			// PlacementNew 사용시 반환할 때 마다 소멸자 호출
			if (_bPlacementNewFlag)
			{
				pData->~T();
			}

			// 노드 반환
			AddNode(pReturn);

			// 사용 중인 노드 수 감소
			_InterlockedDecrement(&_useCount);

			return true;
		}

		inline long GetCapacity() const { return _capacity; }
		inline long GetUseCount() const { return _useCount; }

	private:
		void CreateNode()
		{
			// 메모리 사이즈에 따른 메모리 할당 방식 선택
			Node* pNew;
			constexpr auto DataSize = sizeof(T);
			if (DataSize > 64)
			{
				pNew = static_cast<Node*>(_aligned_malloc(sizeof(Node), 64));
			}
			else
			{
				pNew = static_cast<Node*>(malloc(sizeof(Node)));
			}

#ifdef _SAFE_MODE
			// 안전 모드에서는 새로 할당된 메모리가 해당 메모리풀의 메모리인지 확인할 수 있는 코드를 세팅
			pNew->pFrontGuard = _code;
			pNew->pRearGuard = _code;
#endif
			//Node* pNew = static_cast<Node*>(malloc(sizeof(Node)));
			pNew->bInitializeFlag = false;
			pNew->pNext = nullptr;

			// PlacementNewFlag가 false라면 메모리 최초 할당 시점에 생성자 한 번 호출.
			if (_bPlacementNewFlag == false)
			{
				new (&pNew->Data) T();
			}

			// 노드 추가
			AddNode(pNew);
		}

		void AddNode(Node* pNode)
		{
			Node* snap_pTopNode;
			while (true)
			{
				snap_pTopNode = _topContainer.pNode;
				pNode->pNext = snap_pTopNode;
				auto _Destination = reinterpret_cast<void* volatile*>(&_topContainer.pNode);
				auto _Exchange = reinterpret_cast<void*>(pNode);
				auto _Comparand = reinterpret_cast<void*>(snap_pTopNode);

				// 현재 pTopNode가 해당 스레드에서 Snap한 pTopNode의 값과 비교 후 같다면, pTopNode를 pNew로 변경 시도
				Node* pOld = static_cast<Node*>(_InterlockedCompareExchangePointer(_Destination, _Exchange, _Comparand));

				// InterlockedCompareExchangePointer의 리턴 값은 변경 전의 값이므로, 해당 스레드가 Snap한 pTopNode와 같다면, 변경 성공했으므로 탈출
				if (pOld == snap_pTopNode)
				{
					break;
				}

				_mm_pause();
			}
		}

	private:
		alignas(64) TopContainer	_topContainer = {};
		alignas(64) volatile long	_useCount = 0;
		alignas(64) volatile long	_capacity = 0;
		bool						_bPlacementNewFlag = false;
		char*						_code = nullptr;
	};
}