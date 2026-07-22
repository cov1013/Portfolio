#pragma once
#include "MemoryPool.h"

namespace cov1013
{
	template <typename DATA>
	class ConcurrentStack
	{
		struct Node
		{
			DATA Data = {};
			Node* pNextNode = nullptr;
		};

		struct TopNode
		{
			Node* pNode = nullptr;
			unsigned long long Key = 0;
		};

	public:
		~ConcurrentStack()
		{
			Node* pTopNode = nullptr;

			while (_topNode.pNode != nullptr)
			{
				pTopNode = _topNode.pNode;
				_topNode.pNode = _topNode.pNode->pNextNode;
				_memoryPool.Free(pTopNode);
				_capacity--;
			}

			_capacity = 0;
		}

		void Push(DATA Data)
		{
			Node* pNewNode = _memoryPool.Alloc();
			pNewNode->Data = Data;

			Node* pTopNode;
			do
			{
				pTopNode = _topNode.pNode;
				pNewNode->pNextNode = pTopNode;
			} while (InterlockedCompareExchange64(
				(volatile LONG64*)&_topNode,
				(LONG64)pNewNode,
				(LONG64)pTopNode) != (LONG64)pTopNode);

			_InterlockedIncrement(&_capacity);
		}

		bool Pop(DATA* pDestination)
		{
			if (_InterlockedDecrement(&_capacity) < 0)
			{
				_InterlockedIncrement(&_capacity);
				return false;
			}

			TopNode topNode = {};
			topNode.Key = _topNode.Key;
			topNode.pNode = _topNode.pNode;
			while (!_InterlockedCompareExchange128(
				(volatile LONG64*)&_topNode,
				(LONG64)topNode.Key + 1,
				(LONG64)topNode.pNode->pNextNode,
				(LONG64*)&topNode)) {};

			*pDestination = topNode.pNode->Data;
			_memoryPool.Free(topNode.pNode);

			return true;
		}

		inline const long GetCapacity() const { return _capacity; };

	private:
		MemoryPool<Node> _memoryPool = MemoryPool<Node>(0, false);
		alignas(64)	TopNode _topNode = {};
		alignas(64)	volatile long _capacity = 0;
	};
}