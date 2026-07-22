#pragma once
class Camera
{
public:
	Camera(PlayerObject** pPlayer, const DWORD dwWidth, const DWORD dwHeight);
	~Camera();

	DWORD GetPosX(void);
	DWORD GetPosY(void);
	DWORD GetWidth(void);
	DWORD GetHeight(void);

	void Update(void);

	static Camera* GetInstance();

private:
	static Camera* m_pInstance;
	PlayerObject** m_pPlayer;

	DWORD m_dwWidth;
	DWORD m_dwHeight;
	int m_iPosX;
	int m_iPosY;
};

