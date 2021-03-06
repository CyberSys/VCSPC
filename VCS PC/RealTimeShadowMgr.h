#ifndef __REALTIMESHADOWMGR
#define __REALTIMESHADOWMGR

#define NUM_MAX_REALTIME_SHADOWS		56

#define SHADOWS_MAX_INTENSITY			100

struct tShadowQualitySettings
{
	unsigned char		nQuality, nQualityAfterResample;
	unsigned char		nBlurPasses;
	bool				bUseGradient, bDedicatedCamsForPlayer;
};

class CShadowCamera
{
public:
	RwCamera*		m_pCamera;
	RwTexture*		m_pTexture;

public:
	CShadowCamera()
		: m_pCamera(nullptr), m_pTexture(nullptr)
	{}

	RwCamera*		Create(int nSize);
	void			Destroy();
	RwCamera*		SetLight(RpLight* pLight);
	void			MakeGradientRaster();
	RwCamera*		SetCenter(RwV3d* pVector);
	RwRaster*		RasterResample(RwRaster* pRaster);
	RwRaster*		RasterBlur(RwRaster* pRaster, int nPasses);
	RwRaster*		RasterGradient(RwRaster* pRaster);
	void			InvertRaster();

	RwCamera*		Update(RpAtomic* pAtomic);
	RwCamera*		Update(RpClump* pClump, CEntity* pEntity);
	void			ReInit();
};

class CRealTimeShadow
{
public:
	CEntity*		m_pEntity;
	bool			m_bRenderedThisFrame;
	unsigned char	m_nIntensity;
	bool			m_bUsePlayerHelperCams;		// VCS PC class extension
	CShadowCamera	m_Camera;
	bool			m_bDrawResample;
	CShadowCamera	m_ResampledCamera;
	int				m_dwBlurPasses;
	bool			m_bDrawGradient;
	signed int		m_nRwObjectType;
	RpLight*		m_pLight;
	RwSphere		m_BoundingSphere;
	RwSphere		m_BaseSphere;

public:
	inline RwSphere*			GetBaseSphere()
		{ return &m_BaseSphere; }
	inline class CEntity*		GetOwner()
		{ return m_pEntity; }
	inline bool					GetRenderedThisFrame()
		{ return m_bRenderedThisFrame; }
	inline void					SetRenderedThisFrame()
		{ m_bRenderedThisFrame = true; }
	inline void					ResetIntensity()
		{ m_nIntensity = 0; }
	inline void					ForceFullIntensity()
		{ m_nIntensity = 100; }
	inline void					ClearOwner()
		{ m_pEntity = nullptr; }
	//void*			operator new(size_t size)
	//	{ return GtaOperatorNew(size); }

	CRealTimeShadow(RpLight* pLight)
		:	m_pEntity(nullptr), m_nRwObjectType(-1), m_pLight(pLight), m_dwBlurPasses(0), 
			m_bDrawResample(false), m_bDrawGradient(false), m_bRenderedThisFrame(false), m_nIntensity(0),
			m_bUsePlayerHelperCams(false)
	{}

	~CRealTimeShadow()
	{
		Destroy();
	}

	void			ReInit()
	{
		m_Camera.ReInit();
		m_ResampledCamera.ReInit();
		if ( m_pEntity )
			m_pEntity->bIveBeenRenderedOnce = false;
	}

	void			Destroy();

	void			Create(int nSize, int nSizeResampled, bool bResample, int nBlurPasses, bool bGradient, bool bUsePlayerCams);
	RwTexture*		Update();
	bool			SetShadowedObject(CEntity* pObject);
};


class CRealTimeShadowManager
{
public:
	bool					m_bInitialised;
	bool					m_bNeedsReinit;
	bool					m_bNewSettings;
	bool					m_bPlayerHelperCamsInUse;	// VCS PC class extension
	CRealTimeShadow*		m_pShadows[NUM_MAX_REALTIME_SHADOWS];
	CShadowCamera			m_BlurCamera;
	CShadowCamera			m_GradientCamera;

	// VCS PC class extension
	CShadowCamera			m_BlurCamera_Player;
	CShadowCamera			m_GradientCamera_Player;

	void*	m_pShadowPixelShader;
	RpLight*				m_pGlobalLight;

public:
	CRealTimeShadowManager()
		: m_bInitialised(false), m_bNeedsReinit(false), m_bPlayerHelperCamsInUse(false), m_bNewSettings(false), m_pShadowPixelShader(nullptr)
	{
		memset(m_pShadows, 0, sizeof(m_pShadows));
	}

	inline void*			GetShadowPixelShader()
		{ return m_pShadowPixelShader; }

	void					ResetForChangedSettings()
		{ m_bNeedsReinit = true; m_bNewSettings = true; }

	void					Update();

	void					ReturnRealTimeShadow(CRealTimeShadow* pShadow);
	void					Init();
	void					Exit();
	void					DoShadowThisFrame(CEntity* pEntity);
	void					GetRealTimeShadow(CEntity* pEntity);	
	void					ReInit();
	void					InitShaders();
	void					ReleaseShaders();

	void					KeepBuildingShadowsAlive();
};

RpAtomic* ShadowCameraRenderCB(RpAtomic* pAtomic, void* pData);
RpAtomic* ShadowCameraRenderCB_Vehicle(RpAtomic* pAtomic, void* pData);

extern CRealTimeShadowManager	g_realTimeShadowMan;

static_assert(sizeof(CRealTimeShadow) == 0x4C, "Wrong size: CRealTimeShadow");
//static_assert(sizeof(CRealTimeShadowManager) == 0x54, "Wrong size: CRealTimeShadowManager");

#endif