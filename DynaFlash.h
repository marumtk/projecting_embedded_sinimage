/********************************************************************************
 * �t�@�C�����@�FDynaFlash.h
 * �t�@�C�������FDynaFlash�N���X�̌��J�p��`
 *
 * �C���X�^���X�̐����ɂ́uCreateCommunicationConfig�v�֐����g�p���A
 * �C���X�^���X�̔j���ɂ́uReleaseCommunicationConfig�v�֐����g�p���܂��B
 *
 *--------------------------------------------------------------------------------
 * 2016/12/07 TED �n�� �V�K�쐬
 ********************************************************************************/

#ifndef _DYNAFLASH_H_
#define _DYNAFLASH_H_

#ifdef DYNAFLASH_EXPORTS
#define DYNAFLASH_API __declspec(dllexport) 
#else
#define DYNAFLASH_API __declspec(dllimport) 
#endif

/********************************************************************************
 * �֐��߂�l��`
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
 * �e���`
 ********************************************************************************/

#define MAXIMUM_NUMBER_OF_DYNAFLASH	(4)				/* DynaFlash�ő�ڑ��� */

#define FRAME_BUF_SIZE_8BIT		(1024 * 768)		/* ���e�摜�T�C�Y(Byte) */
#define FRAME_BUF_SIZE_BINARY	(1024 * 768 / 8)	/* ���e�摜�T�C�Y(Byte) */

typedef unsigned int BIT;
typedef enum class PROJECTION_MODE{
    MIRROR = 0x00000001,/* ���e�摜�̍��E���] */
    FLIP = 0x00000002,/* ���e�摜�̏㉺���] */
    COMP = 0x00000004,/* �s�N�Z���f�[�^��bit���] */
    ONESHOT = 0x00000008,/* �����V���b�g���e���[�h */
    BINARY = 0x00000010,/* �o�C�i�����[�h */
    EXT_TRIGGER = 0x00000020,/* �O���g���K�[���[�h */
    TRIGGER_SKIP = 0x00000040,/* �g���K�[�X�L�b�v���[�h */
    PATTERN_EMBED = 0x00000080,/* 1bit�p�^�[�����ߍ��݃��[�h */
    ILLUMINANCE_HIGH = 0x00000100/* ���Ɠx���[�h */
}PM;


/* �Ɠx�ݒ��` */
typedef enum {
	LOW_MODE = 0,									/* ��Ɠx */
	HIGH_MODE										/* ���Ɠx */
} ILLUMINANCE_MODE;

/********************************************************************************
 * �\���̒�`
 ********************************************************************************/

/* �V�X�e���p�����[�^�擾�p�\���� */
typedef struct _tagDynaFlashStatus
{
	unsigned long	Error;							/* DynaFlas�̃G���[��� */
	unsigned long	InputFrames;					/* DynaFlash�֓]�������t���[���� */
	unsigned long	OutputFrames;					/* ���e�ς݂̃t���[���� */
} DYNAFLASH_STATUS, *PDYNAFLASH_STATUS;

/********************************************************************************
 * DynaFlash�N���X�̒�`
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
    // �ǉ��֐�
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

	/* �ȉ����i�łł͖����J�̊֐� */
	virtual int WriteRegister(unsigned int nBar, unsigned int nOffset, unsigned long nData) = 0;
	virtual int ReadRegister(unsigned int nBar, unsigned int nOffset, unsigned long *pData) = 0;

	virtual int WriteDACRegister(unsigned long nIndex, unsigned long nData) = 0;
	virtual int ReadDACRegister(unsigned long nIndex, unsigned long *pData) = 0;
};

/********************************************************************************
 * �C���X�^���X�����֐�
 ********************************************************************************/
DYNAFLASH_API CDynaFlash * _stdcall CreateDynaFlash(void);

/********************************************************************************
 * �C���X�^���X�j���֐�
 ********************************************************************************/
DYNAFLASH_API bool _stdcall ReleaseDynaFlash(CDynaFlash **pDynaFlash);

#endif
