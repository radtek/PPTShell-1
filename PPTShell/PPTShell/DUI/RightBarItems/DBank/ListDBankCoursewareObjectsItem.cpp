#include "stdafx.h"
#include "ListDBankCoursewareObjectsItem.h"
#include "NDCloud/NDCloudFile.h"
#include "NDCloud/NDCloudAPI.h"
#include "DUI/GroupExplorer.h"
#include "Util/FileTypeFilter.h"

CListDBankCoursewareObjectsItem::CListDBankCoursewareObjectsItem()
{

}

CListDBankCoursewareObjectsItem::~CListDBankCoursewareObjectsItem()
{

}

void CListDBankCoursewareObjectsItem::DoInit()
{
	__super::DoInit();

	m_nType = DBankCoursewareObjects;

	m_vecSupportType.push_back(LocalFileInteractiveExercises);
	m_vecSupportType.push_back(FILE_FILTER_INTERACTIVE_EXERCISES);
}

bool CListDBankCoursewareObjectsItem::OnDownloadDecodeList( void* pObj )
{
	CGroupExplorerUI * pGroupExplorer = CGroupExplorerUI::GetInstance();
	THttpNotify* pHttpNotify = (THttpNotify*)pObj;

	if (pHttpNotify->dwErrorCode > 0)
	{
		pGroupExplorer->StopMask();
		pGroupExplorer->ShowNetlessUI(true);
		return true;
	}
	else
	{
		pGroupExplorer->ShowNetlessUI(false);
	}

	if(m_pStream)
		delete m_pStream;
	m_pStream = new CStream(1024);

	if (!NDCloudDecodeCoursewareObjectsList(pHttpNotify->pData, pHttpNotify->nDataSize, m_pStream))
	{
		pGroupExplorer->ShowNetlessUI(true);
		pGroupExplorer->StopMask();
		return false;
	}

	if(IsSelected())
	{
		pGroupExplorer->ShowResource(m_nType, m_pStream);
		pGroupExplorer->StopMask();
	}
	return true;
}

bool CListDBankCoursewareObjectsItem::OnEventLoginComplete( void* pObj )
{
	DWORD dwUserId = NDCloudUser::GetInstance()->GetUserId();
	SetJsonUrl(NDCloudComposeUrlCoursewareObjectsInfo(_T(""), _T(""), 0, 500, dwUserId));
	SetTotalCountUrl(NDCloudComposeUrlCoursewareObjectsInfo(_T(""), _T(""), 0, 500, dwUserId));

	m_pTextTotalCount->SetVisible(false);
	// 
	m_pTotalLoadGif->SetVisible(true);
	m_pTotalLoadGif->PlayGif();

	GetTotalCountInterface();
	return true;
}

bool CListDBankCoursewareObjectsItem::OnGetTotalCount( void * pParam )
{
	THttpNotify * pNotify = (THttpNotify*)pParam;

	if(m_pTotalLoadGif)
	{
		m_pTotalLoadGif->StopGif();
		m_pTotalLoadGif->SetVisible(false);
	}

	int nCount = NDCloudDecodeExercisesCount(pNotify->pData, pNotify->nDataSize);

	TCHAR szBuff[128] ={0};
	_stprintf(szBuff,_T("%d"), nCount);
	CDuiRect rcCalc;
	rcCalc.right = 50;
	CalcText(GetManager()->GetPaintDC(), rcCalc, szBuff, m_pTextTotalCount->GetFont(), DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL | DT_LEFT|DT_NOPREFIX, UIFONT_GDI);
	int nLen = rcCalc.GetWidth();

	m_pTextTotalCount->SetVisible(true);
	m_pTextTotalCount->SetText(szBuff);
	m_pTextTotalCount->SetFixedWidth(nLen);

	m_bTotalNetLess = false;

	return true;
}
