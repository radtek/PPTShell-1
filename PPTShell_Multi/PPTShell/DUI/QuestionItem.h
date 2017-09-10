#pragma once
#include "DUICommon.h"
#include "PhotoItem.h"


class CQuestionItemUI : public CPhotoItemUI
{
public:
	CQuestionItemUI();
	~CQuestionItemUI();

	virtual	void		Init();

	virtual void		OnButtonClick(int nButtonIndex, TNotifyUI& msg );

	virtual	void		OnItemClick(TNotifyUI& msg);

	virtual void		ReadStream( CStream* pStream );

	virtual void		DownloadResource( int nButtonIndex, int nType, int nThumbnailSize );

	virtual void		OnDownloadResourceCompleted( int nButtonIndex, LPCTSTR lptcsPath );

	virtual void		OnItemDragFinish();

	virtual void		OnDownloadThumbnailFailed();

	virtual	void		Delete(int currentModeType);

	void DownloadLocal();//���ص�����
protected:

	tstring				m_strDesc;
	tstring				m_strQuestionName;
	tstring				m_strPreviewUrl;
	tstring				m_strXmlUrl;
	
private:
	bool OnDownloadResourceCompleted2( void* pNotify );
	
};
