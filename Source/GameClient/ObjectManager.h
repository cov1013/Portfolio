#pragma once
class ObjectManager
{
private:
	int					m_iObjectCount;
	List<BaseObject*> m_cObjectList;

public:
	ObjectManager();
	~ObjectManager();

	void Update();
	void Render(BYTE* bypDest, int iDestWidth, int iDestHeight, int iDestPitch);
	void DestroyProc();
	void AscSortY();
	BOOL AddObject(BaseObject* cObject);
	BOOL DeleteObject(int iObjectID);
	BaseObject* FindObject(int iObjectID);

	// 공격 방향/사거리 안의 가장 가까운 플레이어 ObjectID 를 반환한다. (없으면 0)
	int FindAttackTarget(const int AttackerX, const int AttackerY, const bool bFacingLeft,
		const int RangeX, const int RangeY, const int ExcludeID);
};

