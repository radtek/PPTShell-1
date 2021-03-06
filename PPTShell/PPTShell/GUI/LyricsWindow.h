// ========================================================
// 文件:LyricsWindow.h
// 类名:CLyricsWindow
// 功能:歌词显示窗口
// 描述:使用GDIPlus绘制歌词
// ========================================================
// 接口说明
// Create				创建歌词窗口 
// UpdateLyrics			更新歌词(歌词文本,高亮进度百分比)
// Draw					重画歌词窗口
// SetLyricsColor		设置歌词颜色
// SetLyricsBorder		设置歌词边框
// SetHighlightColor	设置高亮歌词颜色
// SetHighlightBorder	设置高亮歌词边框
// SetLyricsShadow		设置歌词阴影
// SetLyricsFont		设置歌词字体
// ========================================================

#pragma once

#include <gdiplus.h>
#pragma comment(lib,"GdiPlus.lib")
using namespace Gdiplus; 

// CWordWindow
typedef struct tagWordInfo
{
	LPWSTR lpszWord;	//显示字体
	int	row;			//显示的行
	RectF dstRect;		//显示位置
	int	speed;			//移动速度
	BOOL DeleteMark;	//移除标志
	BOOL isFirstTime;	//是否定时器第一次走入
} WordInfo, *PWordInfo,FAR *LPWordInfo;

enum GoWordModel
{
	LeftTurn = 0,
	RightTurn = 1
};

//歌词渐变模式
enum LyricsGradientMode
{
	LyricsGradientMode_None		=0, //无渐变
	LyricsGradientMode_Two		=1, //两色渐变
	LyricsGradientMode_Three	=2 //三色渐变
};

// CLyricsWindow

class CLyricsWindow : public CWnd
{
	DECLARE_DYNAMIC(CLyricsWindow)

public:
	CLyricsWindow();
	virtual ~CLyricsWindow();
public:
	////创建窗口 
	BOOL Create(HWND hParentWnd = NULL);
public:
	//更新歌词(歌词文本,高亮进度百分比)
	void UpdateLyrics(LPCSTR lpszLyrics,int nHighlight);
	void UpdateLyrics(LPCWSTR lpszLyrics,int nHighlight);
	//更新高亮进度(高亮进度百分比)
	void UpdateLyrics(int nHighlight);
	//重画歌词窗口
	void Draw();
	//设置歌词颜色
	void SetLyricsColor(Gdiplus::Color TextColor1);
	void SetLyricsColor(Gdiplus::Color TextColor1,Gdiplus::Color TextColor2,LyricsGradientMode TextGradientMode);
	//设置歌词边框
	void SetLyricsBorder(Gdiplus::Color BorderColor,REAL BorderWidth = 1.0f);
	//设置高亮歌词颜色
	void SetHighlightColor(Gdiplus::Color TextColor1);
	void SetHighlightColor(Gdiplus::Color TextColor1,Gdiplus::Color TextColor2,LyricsGradientMode TextGradientMode);
	//设置高亮歌词边框
	void SetHighlightBorder(Gdiplus::Color BorderColor,REAL BorderWidth = 1.0f);
	//设置歌词阴影
	void SetLyricsShadow(Gdiplus::Color ShadowColor,int nShadowOffset=2);
	//设置歌词字体
	void SetLyricsFont(const WCHAR * familyName,REAL emSize,INT style=FontStyleRegular,Unit unit=UnitPixel);
protected:
	//绘制歌词
	void DrawLyrics(Gdiplus::Graphics* pGraphics);
	//绘制高亮歌词
	void DrawHighlightLyrics(Gdiplus::Graphics* pGraphics,Gdiplus::GraphicsPath* pPath,RectF& dstRect);
	//创建渐变画刷
	Gdiplus::Brush* CreateGradientBrush(LyricsGradientMode TextGradientMode,Gdiplus::Color& Color1,Gdiplus::Color& Color2,RectF& dstRect);
	//注册窗口类
	BOOL RegisterWndClass(LPCTSTR lpszClassName);
protected:
	DECLARE_MESSAGE_MAP()
private:
	HDC m_hCacheDC;//缓存DC
	int m_nWidth;
	int m_nHeight;
	LPWSTR m_lpszLyrics;//Unicode格式的歌词
	int m_nHighlight;//高亮歌词的百分比 0--100
	Gdiplus::Color m_TextColor1;//普通歌词颜色,ARGB颜色
	Gdiplus::Color m_TextColor2;//普通歌词颜色,ARGB颜色
	LyricsGradientMode m_TextGradientMode;//普通歌词渐变模式
	Gdiplus::Pen* m_pTextPen;//普通歌词边框画笔
	Gdiplus::Color m_HighlightColor1;//高亮歌词颜色,ARGB颜色
	Gdiplus::Color m_HighlightColor2;//高亮歌词颜色,ARGB颜色
	LyricsGradientMode m_HighlightGradientMode;//高亮歌词渐变模式
	Gdiplus::Pen*m_pHighlightPen;//高亮歌词边框画笔
	Gdiplus::Brush* m_pShadowBrush;//阴影画刷,GDIPlus画刷 
	int m_nShadowOffset;//阴影偏移
	int m_FontStyle;
	REAL m_FontSize;
	Gdiplus::FontFamily* m_pFontFamily;
	Gdiplus::StringFormat* m_pTextFormat;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	// 虚函数 绘制窗口
	Gdiplus::Font* m_pFont;//GDIPlus字体

	CArray<LPWordInfo, LPWordInfo> m_WordInfoArray;	//存放弹幕词语的链表

	void AddWord(LPWordInfo lpWordInfo);
	void AddString(LPWSTR lpWstrWord);	  //添加弹幕词语
	void AddString(LPWSTR lpWstr, int pos, int speed);
	int GetRndNum(int nMin,int nMax); //获取一个范围内的随机数
	void GoWords(GoWordModel goWordModel);	//播放弹幕词语
	void StopWords();						//停止播放弹幕词语
	void Draw2();

	GoWordModel m_goWordModel;	//弹幕方向
	int m_emSize;				//字体大小
	BOOL m_StopFlag;			//停止标志
	

	CRITICAL_SECTION g_cs;


};

