#include "framework.h"
#include "resource.h"
#include "../NetLib/List.h"
#include "../Common/protocol.h"
#include "BaseObject.h"		
#include "PlayerObject.h"		
#include "EffectObject.h"		
#include "ObjectManager.h"	

ObjectManager::ObjectManager()
{
	m_iObjectCount = 0;
}

ObjectManager::~ObjectManager()
{
	List<BaseObject*>::iterator iter = m_cObjectList.begin();
	for (; iter != m_cObjectList.end();)
	{
		delete (*iter);

		iter = m_cObjectList.erase(iter);
	}
}

BOOL ObjectManager::AddObject(BaseObject* cObject)
{
	if (NULL == cObject)
	{
		return FALSE;
	}

	m_cObjectList.push_back(cObject);
	++m_iObjectCount;

	return TRUE;
}

BOOL ObjectManager::DeleteObject(int iObjectID)
{
	List<BaseObject*>::iterator iter = m_cObjectList.begin();
	for (iter; iter != m_cObjectList.end();)
	{
		if ((*iter)->GetObjectID() == iObjectID)
		{
			(*iter)->SetObjectID(-1);
			delete (*iter);
			m_cObjectList.erase(iter);

			--m_iObjectCount;
			return TRUE;
		}
		else
		{
			++iter;
		}
	}
	return FALSE;
}

BaseObject* ObjectManager::FindObject(int iObjectID)
{
	List<BaseObject*>::iterator iter = m_cObjectList.begin();
	for (iter; iter != m_cObjectList.end();)
	{
		if ((*iter)->GetObjectID() == iObjectID)
		{
			return (*iter);
		}
		else
		{
			++iter;
		}
	}

	return NULL;
}

int ObjectManager::FindAttackTarget(const int AttackerX, const int AttackerY, const bool bFacingLeft,
	const int RangeX, const int RangeY, const int ExcludeID)
{
	int		iTargetID = 0;
	int		iNearestDist = RangeX + 1;	// 가장 가까운 |DiffX|

	List<BaseObject*>::iterator iter = m_cObjectList.begin();
	for (; iter != m_cObjectList.end(); ++iter)
	{
		BaseObject* pObject = (*iter);

		// 플레이어만 대상으로 하고, 공격자 본인은 제외
		if (pObject->GetObjectType() != (int)eObjectType::Actor || pObject->GetObjectID() == ExcludeID)
		{
			continue;
		}

		const int DiffX = pObject->GetCurX() - AttackerX;
		const int DiffY = abs(pObject->GetCurY() - AttackerY);

		bool bHit;
		if (bFacingLeft == true)
		{
			bHit = (DiffX <= 0) && (-DiffX <= RangeX) && (DiffY <= RangeY);
		}
		else
		{
			bHit = (DiffX >= 0) && (DiffX <= RangeX) && (DiffY <= RangeY);
		}

		if (bHit == false)
		{
			continue;
		}

		const int Dist = abs(DiffX);
		if (Dist <= iNearestDist)
		{
			iNearestDist = Dist;
			iTargetID = pObject->GetObjectID();
		}
	}

	return iTargetID;
}

void ObjectManager::DestroyProc()
{
	BOOL bDestoryFlag;
	List<BaseObject*>::iterator iter = m_cObjectList.begin();

	for (; iter != m_cObjectList.end();)
	{
		bDestoryFlag = (*iter)->IsDestroy();

		if (bDestoryFlag)
		{
			delete (*iter);

			iter = m_cObjectList.erase(iter);
			++m_iObjectCount;
		}
		else
		{
			++iter;
		}
	}
}

// ==========================================
// Y ���� �������� ����
// ==========================================
void ObjectManager::AscSortY(void)
{
	List<BaseObject*>::iterator iter1 = m_cObjectList.begin();
	List<BaseObject*>::iterator iter2 = m_cObjectList.begin();

	for (int i = 0; i < m_cObjectList.size() - 1; i++)
	{
		for (int j = 0; j < m_cObjectList.size() - 1 - i; j++)
		{
			++iter2;

			if ((*iter1)->GetObjectType() == (int)eObjectType::Effect)
			{
				m_cObjectList.Swap(iter1.GetNode(), iter2.GetNode());
				iter2 = ++iter2;
				iter1 = --iter1;

				++iter1;

				continue;
			}

			if ((*iter1)->GetCurY() > (*iter2)->GetCurY())
			{
				m_cObjectList.Swap(iter1.GetNode(), iter2.GetNode());

				iter2 = ++iter2;
				iter1 = --iter1;
			}
			++iter1;
		}

		iter1 = m_cObjectList.begin();
		iter2 = m_cObjectList.begin();
	}
}

void ObjectManager::Update()
{
	DestroyProc();

	List<BaseObject*>::iterator iter = m_cObjectList.begin();
	for (; iter != m_cObjectList.end(); ++iter)
	{
		(*iter)->Update();
	}

	AscSortY();
}

void ObjectManager::Render(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch)
{
	List<BaseObject*>::iterator iter = m_cObjectList.begin();
	for (; iter != m_cObjectList.end(); ++iter)
	{
		(*iter)->Render(bypDest, iDestWidth, iDestHeight, iDestPitch);
	}
}
