/********************************************************************************
 * ファイル名　：DynaFlash.h
 * ファイル説明：DynaFlashクラスの公開用定義
 *
 * インスタンスの生成には「CreateCommunicationConfig」関数を使用し、
 * インスタンスの破棄には「ReleaseCommunicationConfig」関数を使用します。
 *
 *--------------------------------------------------------------------------------
 * 2016/12/07 TED 渡瀬 新規作成
 ********************************************************************************/

#ifndef _DYNAFLASH_H_
#define _DYNAFLASH_H_

#ifdef DYNAFLASH_EXPORTS
#define DYNAFLASH_API __declspec(dllexport) 
#else
#define DYNAFLASH_API __declspec(dllimport) 
#endif

/********************************************************************************
 * 関数戻り値定義
 ********************************************************************************/

#define STATUS_SUCCESSFUL		 		(0x0000)
#define	STATUS_INCOMPLETE				(0x0001)
#define	STATUS_INVALID_PARAM			(0x0002)
#define STATUS_INVALID_BOARDINDEX		(0x0004)
#define STATUS_ALLOC_FAILED				(0x0008)
#define STATUS_INVALID_DEVICEHANDLE		(0x0010)
#define STATUS_INVALID_BARNUM			(0x0020)
#define STATUS_LOCK_FAILED				(0x0040)
#define STATUS_UNLOCK_FAILED			(0x0080)
#define STATUS_FREE_FAILED				(0x0100)
#define STATUS_INVALID_CHINDEX			(0x0200)
#define STATUS_DMA_TIMEOUT				(0x0400)
#define STATUS_NO_TRIGIN				(0x0800)

/********************************************************************************
 * 各種定義
 ********************************************************************************/

#define MAXIMUM_NUMBER_OF_DYNAFLASH	(4)				/* DynaFlash最大接続数 */

#define FRAME_BUF_SIZE_8BIT		(1024 * 768)		/* 投影画像サイズ(Byte) */
#define FRAME_BUF_SIZE_BINARY	(1024 * 768 / 8)	/* 投影画像サイズ(Byte) */

typedef unsigned int BIT;
typedef enum class PROJECTION_MODE{
    MIRROR = 0x00000001,/* 投影画像の左右反転 */
    FLIP = 0x00000002,/* 投影画像の上下反転 */
    COMP = 0x00000004,/* ピクセルデータのbit反転 */
    ONESHOT = 0x00000008,/* ワンショット投影モード */
    BINARY = 0x00000010,/* バイナリモード */
    EXT_TRIGGER = 0x00000020,/* 外部トリガーモード */
    TRIGGER_SKIP = 0x00000040,/* トリガースキップモード */
    PATTERN_EMBED = 0x00000080,/* 1bitパターン埋め込みモード */
    ILLUMINANCE_HIGH = 0x00000100/* 高照度モード */
}PM;


/* 照度設定定義 */
typedef enum {
	LOW_MODE = 0,									/* 低照度 */
	HIGH_MODE										/* 高照度 */
} ILLUMINANCE_MODE;

/********************************************************************************
 * 構造体定義
 ********************************************************************************/

/* システムパラメータ取得用構造体 */
typedef struct _tagDynaFlashStatus
{
	unsigned long	Error;							/* DynaFlasのエラー情報 */
	unsigned long	InputFrames;					/* DynaFlashへ転送したフレーム数 */
	unsigned long	OutputFrames;					/* 投影済みのフレーム数 */
} DYNAFLASH_STATUS, *PDYNAFLASH_STATUS;

/********************************************************************************
 * DynaFlashクラスの定義
 ********************************************************************************/
class CDynaFlash
{
public:
	explicit CDynaFlash() {}
	virtual ~CDynaFlash() {}
	
	virtual int Connect(unsigned int nDynaFlashIndex) = 0;
	virtual int Disconnect(void) = 0;
	virtual int PowerOff(void) = 0;
	virtual unsigned int GetIndex(void) = 0;

	virtual int GetDriverVersion(char nVersion[40]) = 0;
	virtual int GetHWVersion(unsigned long *pVersion) = 0;
	virtual int GetDLLVersion(unsigned long *pVersion) = 0;

	virtual int Reset(void) = 0;
	virtual int Start(void) = 0;
	virtual int Stop(void) = 0;
	virtual int Float(unsigned int isPowerFloat) = 0;

	virtual int SetParam(unsigned long nFrameRate, unsigned long nBitDepth, unsigned long nProjectionMode) = 0;
    // 追加関数
    virtual int SetParam(unsigned long nFrameRate, unsigned long nBitDepth, unsigned long *nBitSequence, unsigned long nPatternOffset, int *pnLEDAdjust, unsigned long nProjectionMode) = 0;

	virtual int GetStatus(PDYNAFLASH_STATUS pDynaFlashStatus) = 0;

	virtual int SetIlluminance(ILLUMINANCE_MODE eIlluminanceMode) = 0;
	virtual int GetIlluminance(ILLUMINANCE_MODE *pIlluminanceMode) = 0;

	virtual int AllocFrameBuffer(unsigned long nFrameCnt) = 0;
	virtual int ReleaseFrameBuffer(void) = 0;

	virtual int GetFrameBuffer(char **ppBuffer, unsigned long *pFrameCnt) = 0;
	virtual int PostFrameBuffer(unsigned long nFrameCnt) = 0;

	virtual int GetFpgaInfo(float *pTemp, float *pVaux, float *pVint) = 0;
	virtual int GetFanInfo(unsigned long *pData) = 0;
	virtual int GetLedTemp(float *nTemp) = 0;

	/* 以下製品版では未公開の関数 */
	virtual int WriteRegister(unsigned int nBar, unsigned int nOffset, unsigned long nData) = 0;
	virtual int ReadRegister(unsigned int nBar, unsigned int nOffset, unsigned long *pData) = 0;

	virtual int WriteDACRegister(unsigned long nIndex, unsigned long nData) = 0;
	virtual int ReadDACRegister(unsigned long nIndex, unsigned long *pData) = 0;
};

/********************************************************************************
 * インスタンス生成関数
 ********************************************************************************/
DYNAFLASH_API CDynaFlash * _stdcall CreateDynaFlash(void);

/********************************************************************************
 * インスタンス破棄関数
 ********************************************************************************/
DYNAFLASH_API bool _stdcall ReleaseDynaFlash(CDynaFlash **pDynaFlash);

#endif
