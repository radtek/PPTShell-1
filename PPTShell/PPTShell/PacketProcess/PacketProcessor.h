//===========================================================================
// FileName:				PacketProcessor.h
//	
// Desc:					 process packets
//============================================================================
#ifndef _PACKET_PROCESSOR_H_
#define _PACKET_PROCESSOR_H_

#include "Util/Singleton.h"
#include "DUI/Shape.h"

struct SOCK_PACKET
{
	SOCKET			sock;
	int				type;		// connect type	
	char*			buffer;
};

class CUser;
class CPacketProcessor
{
private:
	CPacketProcessor();
	~CPacketProcessor();

public:
	void Initialize();
	void Destroy();
	void OnProcessPacket(SOCKET sock, int connectType, char* pPacket);
	void OnSocketClosed(SOCKET sock);

	static DWORD WINAPI ProcessPacketThread(LPARAM lParam);
	void ProcessPacket();
	void ProcessOnePacket(SOCK_PACKET* pPacket);

	DECLARE_SINGLETON_CLASS(CPacketProcessor);

protected:
	// control PPT
	void PPTControl(SOCK_PACKET* pSockPacket);
	void ImageControl(SOCK_PACKET* pSockPacket);
	void ThreeDControl(SOCK_PACKET* pSockPacket);
	void LaserControl(SOCK_PACKET* pSockPacket, DWORD pkgLen);
	void BrushControl(SOCK_PACKET* pSockPacket, DWORD pkgLen);
	void MagnifierControl(SOCK_PACKET* pSockPacket);		// 处理放大镜
	void SpotlightControl(SOCK_PACKET* pSockPacket);		// 处理聚光灯
	void BlackBoardControl(SOCK_PACKET* pSockPacket);		// 处理黑板

	void SyncSlideImgRequest(SOCK_PACKET* pSockPacket);
protected:
	void ProcessActionUser(SOCK_PACKET* pSockPacket);
	void ProcessActionControl(SOCK_PACKET* pSockPacket);
	void ProcessActionTransferFile(SOCK_PACKET* pSockPacket);
	void ProcessActionPushData(SOCK_PACKET* pSockPacket);
	void ProcessActionPullData(SOCK_PACKET* pSockPacket);
	void ProcessActionSyncData(SOCK_PACKET* pSockPacket);

	BOOL TransferFileCheckExists(SOCK_PACKET *pSockPacket);
protected:
	BOOL PlayEffect(int nEffectType);
	BOOL ExecCalculator();
	BOOL KillCalculator();
	BOOL ControlCalcTop();//居中置顶显示已经打开的计算器2015.12.21 cws

	int PerFloatToSlideLength(float fPercentt, int nLengthType, bool bIncludeBlank = false);//将float百分比转换成长度

	//BOOL SendResponeToUser(SOCK_PACKET pSockPacket, DWORD dwMajor, DWORD dwMinor, DWORD dwThirdCmd, BOOL bSuccess, tstring& strErrorDesc);
protected:
	HANDLE					m_hEvent;
	HANDLE					m_hProcessThread;	// 处理线程句柄 
	CLineShape*				m_pNewLineShape;//一次动作的shape
	BOOL					m_bCloseApp;		// 程序是否将退出
	CRITICAL_SECTION		m_cs;
	deque<SOCK_PACKET*>		m_ProcessDeque;	 

	map<tstring, DWORD>		m_mapTransferFile;		// 传输的文件列表，KV：文件名-文件CRC
};

typedef Singleton<CPacketProcessor> PacketProcesser;

#endif