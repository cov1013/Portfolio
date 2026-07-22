#pragma once
#include "MemoryPool.h"

namespace cov1013
{
	template <typename DATA>
	class ConcurrentQueue
	{
		struct Node
		{
			DATA	Data;
			Node*	pNextNode;
		};

		struct Container
		{
			Node*				pNode;
			unsigned long long	Key;
		};

	public:
		ConcurrentQueue()
		{
			_head.pNode = _memoryPool.Alloc();	// 더미 노드 생성
			_head.Key = 0;
			_head.pNode->pNextNode = nullptr;

			_tail.pNode = _head.pNode;
			_tail.Key = 0;
			_tail.pNode->pNextNode = nullptr;

			_capacity = 0;
		}

		~ConcurrentQueue()
		{
			_memoryPool.Free(_head.pNode);		// 더미 노드 반환
		}

		void Enqueue(DATA data)
		{
			Node* pNewNode = _memoryPool.Alloc();
			pNewNode->Data = data;
			pNewNode->pNextNode = nullptr;		

			Container tail = {};
			Node* pTailNextNode;

			while (true)
			{
				tail.Key = _tail.Key;
				tail.pNode = _tail.pNode;
				pTailNextNode = tail.pNode->pNextNode;

				if (pTailNextNode == nullptr)
				{
					if (_InterlockedCompareExchange64(
						(volatile LONG64*)&_tail.pNode->pNextNode,
						(LONG64)pNewNode, 
						(LONG64)pTailNextNode) == (LONG64)pTailNextNode)
					{
						//---------------------------------------------------------------------
						// 실패 1. 현재 tailNext가 NULL이므로 노드를 추가했지만, SnapKey가 현재 Key와 다른 경우.
						// 실패 2. Snaptail이 현재 tail이 아닌 경우.
						//---------------------------------------------------------------------
						_InterlockedCompareExchange128(
							(volatile LONG64*)&_tail, 
							(LONG64)tail.Key + 1, 
							(LONG64)pNewNode, 
							(LONG64*)&tail);
						break;
					}
				}
				else
				{
					//---------------------------------------------------------------------
					// tailNext가 NULL이 아니면 한 칸 밀고 다시 시도
					// 밀지 않고 그냥 Enqueue하면 기존 tailNext 메모리 유실 + Capacity와 실제 노드 개수가 일치하지 않음.
					//---------------------------------------------------------------------
					_InterlockedCompareExchange128(
						(volatile LONG64*)&_tail, 
						(LONG64)tail.Key + 1, 
						(LONG64)pTailNextNode,
						(LONG64*)&tail);
				}
			}

			_InterlockedIncrement(&_capacity);
		}

		bool Dequeue(DATA* pDestination)
		{
			if (_InterlockedDecrement(&_capacity) < 0)
			{
				_InterlockedIncrement(&_capacity);
				return false;
			}

			Container tail = {};
			Container head = {};
			Node* pHeadNextNode = nullptr;
			Node* pTailNextNode = nullptr;

			while (true)
			{
				head.Key = _head.Key;
				head.pNode = _head.pNode;
				pHeadNextNode = head.pNode->pNextNode;

				//---------------------------------------------------------------------
				// [Enqueue 실패2]의 경우 Snaptail이라고 생각한 노드 뒤에 할당받은 노드를 삽입한 후 Capacity를 증가한 후 나가는데,
				// 그 Capacity를 인식하고 들어왔다. 하지만 해당 노드는 실제 큐에 적용되기 전인 상황. 결국 큐에 적용될 것이므로, 적용되기 전 까지 계속 시도.
				// 
				// 해당 과정을 무시하면 headNext가 NULL인 경우를 참조하게 된다.
				//---------------------------------------------------------------------
				if (pHeadNextNode == nullptr)
				{
					continue;
				}
				else
				{
					//---------------------------------------------------------------------
					// 실제 출력할 노드가 있어도, tailNext가 NULL이 아닌 상태에서 출력하고 head를 갱신하면
					// head가 tail을 추월하는 상황이 발생된다. 이 경우 tail 노드가 반환되는 경우가 생기고
					// tail의 Next가 tail이 되는 상황과 Capacity가 실제 노드 수와 다른 상황이 발생한다.
					// 
					// 위 상황의 경우, Enqueue에서 tail의 Next를 계속 tail로 갱신하면서 빠져나오지 못하는 상황 발생하고,
					// Dequeue에서는 pHeadNextNode가 계속 NULL이므로 빠져나오지 못하는 상황이 발생한다.
					//---------------------------------------------------------------------
					tail.Key = _tail.Key;
					tail.pNode = _tail.pNode;
					pTailNextNode = tail.pNode->pNextNode;
					if (pTailNextNode != nullptr)
					{
						_InterlockedCompareExchange128(
							(volatile LONG64*)&_tail, 
							(LONG64)tail.Key + 1, 
							(LONG64)pTailNextNode, 
							(LONG64*)&tail);
					}
				}

				//---------------------------------------------------------------------
				// head 갱신 후 데이터를 출력하게 되면, pHeadNextNode가 head가 되면서 다른 스레드의 접근 대상이 된다.
				// 즉, 내가 알고있던 pHeadNextNode의 값이 다른 값으로 변경되는 상황이 발생한다.
				//---------------------------------------------------------------------
				*pDestination = pHeadNextNode->Data;
				if (_InterlockedCompareExchange128(
					(volatile LONG64*)&_head,
					(LONG64)head.Key + 1, 
					(LONG64)pHeadNextNode,
					(LONG64*)&head))
				{
					_memoryPool.Free(head.pNode);
					return true;
				}
			}
		}

		inline const long GetCapacity() const { return _capacity; }

	private:
		MemoryPool<Node> _memoryPool = MemoryPool<Node>(0, false);
		alignas(64) Container _head;
		alignas(64) Container _tail;
		alignas(64) long _capacity;
	};
}