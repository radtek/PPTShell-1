#include "StdAfx.h"
#include "LocalCourseItem.h"
#include "ItemExplorer.h"
#include "NDCloud/NDCloudAPI.h"
#include "NDCloud/NDCloudResModel.h"
#include "PPTControl/PPTController.h"
#include "PPTControl/PPTControllerManager.h"
#include "PPTControl/PPTImagesExporter.h"
#include "Util/Util.h"
#include "Statistics/Statistics.h"
#include "DUI/ItemExplorer.h"
#include "DUI/GroupExplorer.h"


CLocalCourseItemUI::CLocalCourseItemUI()
{
	m_dwExportId = 0;
}

CLocalCourseItemUI::~CLocalCourseItemUI()
{
	PPTImagesExporter::GetInstance()->CancelExport(m_dwExportId);
	CStream* pStream = (CStream*)this->GetTag();
	if (pStream)
	{
		delete pStream;
		this->SetTag(NULL);
	}
	m_dwUploadServerPathTaskId = 0;
	m_dwUploadNetdiscTaskId = 0;
	m_dwUplpadCommitTaskId = 0;
}

void CLocalCourseItemUI::Init()
{
	__super::Init();
	SetContentHeight(110);
	CControlUI* pCtrl = FindSubControl(_T("btn2"));
	if (pCtrl)
	{
		pCtrl->SetVisible(false);
	}
	SetImage(_T("RightBar\\Item\\item_bg_course.png"));

	SetIcon(_T("RightBar\\Item\\bg_tit_class.png"));
	
}

LPCTSTR CLocalCourseItemUI::GetButtonText( int nIndex )
{
	if (nIndex == 0)
	{
		return _T("应用");
	}

	return __super::GetButtonText(nIndex);

}

void CLocalCourseItemUI::OnButtonClick( int nButtonIndex, TNotifyUI& msg )
{
	if( !IsResourceExist() )
	{
		CToast::Toast(_STR_LOCAL_ITEM_TIP_FILE_NO_EXIST, false, 1000);
		return;
	}

	if (nButtonIndex == 0)
	{
		InsertPPTByThread(GetResource(), 1, -1);
		Statistics::GetInstance()->Report(STAT_INSERT_PPT);
	}
	else if(nButtonIndex == 1)
	{
		OpenAsDefaultExec(GetResource());
	}
	
}


void CLocalCourseItemUI::OnItemClick( TNotifyUI& msg )
{

	CStream* pStream = (CStream*)this->GetTag();
	if (pStream)
	{
		CItemExplorerUI::GetInstance()->ShowWindow(true);
		CItemExplorerUI::GetInstance()->ResetUI();
		CItemExplorerUI::GetInstance()->ShowResource(eItemExplorer_Local, pStream, this);
		CItemExplorerUI::GetInstance()->StopMask();
	}
	else
	{
		if ( !GetPPTController()->IsInit() )
		{
			CToast::Toast(_STR_PPTCTRLMGR_TIP_PPT_NOT_OPEN, true);
		}
		else
		{
			CItemExplorerUI::GetInstance()->ShowWindow(true);
			CItemExplorerUI::GetInstance()->ResetUI();
			CItemExplorerUI::GetInstance()->StartMask();
		
			m_dwExportId = PPTImagesExporter::GetInstance()->ExportImages(GetResource(), MakeDelegate(this,  &CLocalCourseItemUI::OnExportImagesCompleted));
			if (!m_dwExportId)
			{
				CItemExplorerUI::GetInstance()->StopMask();
			}
		}
	}
}

bool CLocalCourseItemUI::OnExportImagesCompleted( void* pObj )
{
	ExportNotify* pNotify = (ExportNotify*)pObj;
	
	if (!this->IsSelected())
	{
		return true;
	}

	if (pNotify->dwErrorCode != 0)
	{
		CItemExplorerUI::GetInstance()->StartMask();
	}

	CStream* pStream = new CStream(1024);
	pStream->WriteDWORD(pNotify->nCount);
	for (int i = 0; i < pNotify->nCount; ++i)
	{
		TCHAR szFileName[MAX_PATH]	= {0};
		TCHAR szFilePath[MAX_PATH]	= {0};

		_stprintf_s(szFileName, "Slide_%d.%s", i + 1, pNotify->lptcsExt);
		_stprintf_s(szFilePath, "%s\\%s", pNotify->strDir.c_str(), szFileName);

		pStream->WriteString(szFileName);
		pStream->WriteString(szFilePath);
	}
	this->SetTag((UINT_PTR)pStream);

	CItemExplorerUI::GetInstance()->ShowResource(eItemExplorer_Local, pStream, this);
	CItemExplorerUI::GetInstance()->StopMask();
	return true;
}

bool CLocalCourseItemUI::HasSubitems()
{
	return true;
}

void CLocalCourseItemUI::UploadNetdisc(int currentModeType)
{
	DWORD dwUserID = NDCloudUser::GetInstance()->GetUserId();
	if (dwUserID != 0)
	{
		string strResource  = GetResource();
		DWORD dwAttri = ::GetFileAttributes(strResource.c_str());
		if ((dwAttri != -1) && !(dwAttri&FILE_ATTRIBUTE_DIRECTORY))
		{
			string strTitle = GetTitle();
			string strToast = "“"+ strTitle +"”" + "开始上传...";
			CToast::Toast(strToast);
			CHttpDownloadManager* pHttpManager = HttpDownloadManager::GetInstance();
			if( pHttpManager != NULL )
			{
				CStream* pStream=new CStream(1024);
				pStream->WriteString(strTitle);
				pStream->WriteString(strResource);
				pStream->WriteInt(currentModeType);
				TCHAR szPost[1024];
				_stprintf_s(szPost, "/v0.6/coursewares/none/uploadurl?uid=%u&renew=false", dwUserID);
				m_dwUploadServerPathTaskId = pHttpManager->AddTask(_T("esp-lifecycle.web.sdp.101.com"), szPost, _T(""), _T("GET"), _T(""), 80, 
					MakeHttpDelegate(this, &CLocalCourseItemUI::OnGetServerPath),
					MakeHttpDelegate(NULL),
					MakeHttpDelegate(NULL), 
					TRUE,FALSE,0,pStream);
				if(m_dwUploadServerPathTaskId==0)
				{
					strToast = "“"+ strTitle +"”" + "上传失败";
					CToast::Toast(strToast);
				}
			}
		}
		else
		{
			CToast::Toast("文件不存在");
		}
	}
}

bool CLocalCourseItemUI::OnGetServerPath( void * pParam )
{
	THttpNotify* pNotify = static_cast<THttpNotify*>(pParam);
	string strTitle = "";
	string strResource = "";
	int currentModeType=0;
	if(pNotify->pUserData)
	{
		CStream* pStream = (CStream*)pNotify->pUserData;
		if(pStream)
		{
			pStream->ResetCursor();
			strTitle = pStream->ReadString();
			strResource = pStream->ReadString();
			currentModeType = pStream->ReadInt();
			delete pStream;
		}
	}
	bool isSuccess=false;
	if(pNotify->dwErrorCode == 0&&pNotify->nDataSize>0)
	{
		pNotify->pData[pNotify->nDataSize] = '\0';
		tstring str = pNotify->pData;
		Json::Reader reader;
		Json::Value root;
		bool res = reader.parse(str, root);
		if( res )
		{
			tstring strUrl="";
			tstring strSessionId="";
			tstring strServerPath = "";
			tstring strUploadGuid = "";
			if( !root["access_url"].isNull() )
			{
				strUrl = root["access_url"].asCString();
			}
			if( !root["uuid"].isNull() )
			{
				strUploadGuid = root["uuid"].asCString();
			}
			if( !root["session_id"].isNull() )
			{
				strSessionId = root["session_id"].asCString();
			}
			if( !root["dist_path"].isNull() )
			{
				strServerPath = root["dist_path"].asCString();
			}
			if(!strUrl.empty()&&!strUploadGuid.empty()&&!strSessionId.empty()&&!strServerPath.empty())
			{
				isSuccess=true;
				UploadFile(strUrl.c_str(), strSessionId.c_str(), strUploadGuid.c_str(), strServerPath.c_str(),strTitle.c_str(),strResource.c_str(),currentModeType);
			}
		}
	}
	if(!isSuccess)
	{
		string strToast = "“"+ strTitle +"”" + "上传失败";
		CToast::Toast(strToast);
	}
	return true;
}

void CLocalCourseItemUI::UploadFile(LPCTSTR lptcsUrl, LPCTSTR lptcsSessionId, LPCTSTR lptcsUploadGuid, LPCTSTR lptcsServerFilePath,LPCTSTR strTitle,LPCTSTR strResource,int currentModeType)
{
	TCHAR szHost[128];
	TCHAR szUrlPath[1024];
	if(sscanf_s(lptcsUrl, _T("http://%[^/]%s"), szHost, sizeof(szHost) - 1, szUrlPath , sizeof(szUrlPath) - 1) == 2)
	{
		tstring strHost = szHost;
		tstring strUrlPath = szUrlPath;
		strUrlPath += _T("?session=");
		strUrlPath += lptcsSessionId;

		CStream* pStream = new CStream(1024);
		pStream->WriteString(lptcsUploadGuid);
		pStream->WriteString(lptcsServerFilePath);
		pStream->WriteString(strTitle);
		pStream->WriteString(strResource);
		pStream->WriteInt(currentModeType);
		m_dwUploadNetdiscTaskId = HttpUploadManager::GetInstance()->AddUploadTask(strHost.c_str(), 
			strUrlPath.c_str(),
			strResource,
			lptcsServerFilePath,
			_T("POST"), 
			80,
			MakeHttpDelegate(this, &CLocalCourseItemUI::OnUploadCompleted),
			MakeHttpDelegate(NULL),FALSE,FALSE,
			pStream);
		if(m_dwUploadNetdiscTaskId==0)
		{
			string strToast = strTitle;
			strToast = "“"+strToast+"”" + "上传失败";
			CToast::Toast(strToast);
		}
	}
}

bool CLocalCourseItemUI::OnUploadCompleted( void * pParam )
{
	THttpNotify* pHttpNotify = (THttpNotify*)pParam;
	string strServerPath = "";
	string strUploadGuid = "";
	string strTitle = "";
	string strResource = "";
	int currentModeType=0;
	CStream* pStream=NULL;
	tstring strFullFileServerPath;
	if(pHttpNotify->pUserData)
	{
		pStream = (CStream*)pHttpNotify->pUserData;
		if(pStream)
		{
			pStream->ResetCursor();
			strUploadGuid = pStream->ReadString();
			strServerPath = pStream->ReadString();
			strTitle = pStream->ReadString();
			strResource = pStream->ReadString();
			currentModeType = pStream->ReadInt();
		}
	}
	if (pHttpNotify->dwErrorCode==0)
	{
		if(pHttpNotify->nDataSize>0)
		{
			pHttpNotify->pData[pHttpNotify->nDataSize] = '\0';
			tstring str = pHttpNotify->pData;
			Json::Reader reader;
			Json::Value root;

			bool res = reader.parse(str, root);
			if(!res||root.get("dentry_id", Json::Value()).isNull())
			{
				pHttpNotify->dwErrorCode=5;	
			}

			if(res && !root.get("path", Json::Value()).isNull())
			{
				strFullFileServerPath= Ansi2Str(root["path"].asCString());
			}
		}
	}
	if (pHttpNotify->dwErrorCode==0)
	{
		DWORD dwUserId = NDCloudUser::GetInstance()->GetUserId();
		if(dwUserId!=0)
		{
			tstring strChapterGuid = NDCloudGetChapterGUID();
			TCHAR szUserId[MAX_PATH];
			_stprintf_s(szUserId, _T("%d"), dwUserId);

			CNDCloudResourceModel ResModel;

			ResModel.SetGuid(strUploadGuid.c_str());
			string strTmp = strTitle;
			int length = strTmp.length();
			if(length>200)
			{
				strTmp = strTmp.substr(0,200);
			}
			ResModel.SetBasicInfo(strTmp);
			ResModel.SetPreview();
			ResModel.SetTechInfo(strResource, strServerPath.c_str(), strFullFileServerPath);
			ResModel.SetLifeCycleInfo();
			ResModel.AddCoverage(_T("User"), szUserId, _T(""), _T("OWNER") );
			ResModel.AddRelation(strChapterGuid);

			ResModel.AddRequirement(_T("SOFTWARE"), _T("editor"), _T("2007"), _T("2013"), _T("microsoft office 2007+"));
			ResModel.AddRequirement(_T("SOFTWARE"), _T("player"), _T("2007"), _T("2013"), _T("microsoft office 2007+"));

			CCategoryTree* pCategory;
			NDCloudGetCategoryTree(pCategory);
			ResModel.SetCategoryInfo(pCategory);
			m_dwUplpadCommitTaskId = ResModel.CommitResourceModel(MakeHttpDelegate(this, &CLocalCourseItemUI::OnUploadCommitCompleted),pStream);
			if(m_dwUplpadCommitTaskId==0)
			{
				string strToast = "“"+ strTitle +"”" + "上传失败";
				CToast::Toast(strToast);
			}
		}
	}
	else
	{
		string strToast = "“"+ strTitle +"”" + "上传失败";
		CToast::Toast(strToast);
	}
	return true;
}

bool CLocalCourseItemUI::OnUploadCommitCompleted( void * pParam )
{
	THttpNotify* pHttpNotify = (THttpNotify*)pParam;
	string strServerPath = "";
	string strUploadGuid = "";
	string strTitle = "";
	string strResource = "";
	int currentModeType=0;
	if(pHttpNotify->pUserData)
	{
		CStream* pStream = (CStream*)pHttpNotify->pUserData;
		if(pStream)
		{
			pStream->ResetCursor();
			strUploadGuid = pStream->ReadString();
			strServerPath = pStream->ReadString();
			strTitle = pStream->ReadString();
			strResource = pStream->ReadString();
			currentModeType = pStream->ReadInt();
			delete pStream;
		}
	}
	bool success = false;
	if (pHttpNotify->dwErrorCode==0)
	{
		if(pHttpNotify->nDataSize>0)
		{
			pHttpNotify->pData[pHttpNotify->nDataSize] = '\0';
			string strContent = Utf8ToAnsi(pHttpNotify->pData);
			Json::Reader reader;
			Json::Value root;
			bool res = reader.parse(strContent, root);
			if(res)
			{
				if( root["code"].isNull() )
				{
					success = true;
					string strToast = "“"+ strTitle +"”" + "上传成功";
					CToast::Toast(strToast);
				}
			}
		}
	}
	if(!success)
	{
		string strToast = "“"+ strTitle +"”" + "上传失败";
		CToast::Toast(strToast);
	}
	BroadcastEvent(EVT_MENU_UPLOADNETDISC_COMPELETE,(WPARAM)pHttpNotify->dwErrorCode,(LPARAM)currentModeType);
	return true;
}