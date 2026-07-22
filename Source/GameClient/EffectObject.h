#pragma once
class EffectObject : public BaseObject
{
private:
	BOOL			m_bActionFlag;
	DWORD			m_dwAttackID;
	PlayerObject*  m_pPlayer;

public:
	EffectObject();
	EffectObject(PlayerObject* pPlayer);
	virtual ~EffectObject();

	void Update(void);
	void Render(BYTE* bypDest, const int iDestWidth, const int iDestHeight, const int iDestPitch);
};