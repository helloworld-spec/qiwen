// LineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "LineDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace Gdiplus;

#define PI	3.1415926
#define CONTROL_POINT_APART		(LINE_WINDOW_WIDTH / m_level_num)


class Vector;
Vector operator +(Vector & V1, Vector & V2);
Vector operator -(Vector & V1, Vector & V2);
Vector operator -(Vector & V1);
Vector operator *(double c, Vector & V);
Vector operator *(Vector & V, double c);
Vector operator /(Vector & V, double c);

class Vector
{
public:
	double _x, _y;

	Vector(double x, double y)
	{
		_x = x; _y = y;
	};

	Vector(PointF pt)
	{
		_x = pt.X;
		_y = pt.Y;
	};

	Vector(PointF st, PointF end)
	{
		_x = end.X - st.X;
		_y = end.Y - st.Y;
	};

	Vector(Point pt)
	{
		_x = pt.X;
		_y = pt.Y;
	};

	Vector(Point st, Point end)
	{
		_x = end.X - st.X;
		_y = end.Y - st.Y;
	};

	double Magnitude()
	{
		return sqrt(_x * _x + _y * _y);
	};

	friend Vector operator +(Vector & V1, Vector & V2)
	{
		return Vector(V1._x + V2._x, V1._y + V2._y);
	};

	friend Vector operator -(Vector & V1, Vector & V2)
	{
		return Vector(V1._x - V2._x, V1._y - V2._y);
	};

	friend Vector operator -(Vector & V1)
	{
		return Vector(-V1._x, -V1._y);
	};

	friend Vector operator *(double c, Vector & V)
	{
		return Vector(c * V._x, c * V._y);
	};

	friend Vector operator *(Vector & V, double c)
	{
		return Vector(c * V._x, c * V._y);
	};

	friend Vector operator /(Vector & V, double c)
	{
		return Vector(V._x / c, V._y / c);
	};

    // A * B =|A|.|B|.sin(angle AOB)
    double CrossProduct(Vector & V)
    {
		return _x * V._y - V._x * _y;
    };

    // A. B=|A|.|B|.cos(angle AOB)
    double DotProduct(Vector & V)
    {
		return _x * V._x + _y * V._y;
    };

    static bool IsClockwise(PointF pt1, PointF pt2, PointF pt3)
    {
        Vector V21(pt2, pt1);
        Vector v23(pt2, pt3);
        return V21.CrossProduct(v23) < 0; // sin(angle pt1 pt2 pt3) > 0, 0<angle pt1 pt2 pt3 <180
    };

	static bool IsCCW(PointF pt1, PointF pt2, PointF pt3)
    {
        Vector V21(pt2, pt1);
        Vector v23(pt2, pt3);
        return V21.CrossProduct(v23) > 0;  // sin(angle pt2 pt1 pt3) < 0, 180<angle pt2 pt1 pt3 <360
    };

	static double DistancePointLine(PointF pt, PointF lnA, PointF lnB)
    {
        Vector v1(lnA, lnB);
        Vector v2(lnA, pt);
        v1 = v1 / v1.Magnitude();
        return abs(v2.CrossProduct(v1));
    }

	void Rotate(int Degree)
    {
        double radian = Degree * PI / 180.0;
        double dsin = sin(radian);
        double dcos = cos(radian);
        double nx = _x * dcos - _y * dsin;
        double ny = _x * dsin + _y * dcos;
        _x = nx;
        _y = ny;
    }

	Point ToPoint()
    {
        return Point((int)_x, (int)_y);
    }

	PointF ToPointF()
    {
        return PointF((float)_x, (float)_y);
    }
};


typedef enum _condition
{
    NATURAL,
    CLAMPED,
    NOTaKNOT
}condition;
 
typedef struct _coefficient
{
    double a3;
    double b2;
    double c1;
    double d0;
}coefficient;
 
typedef struct _coe_point
{
    coefficient *coe;
    double *xCoordinate;
    double *yCoordinate;
    double f0;
    double fn;
    int num;
    condition con;
}COE_POINT;
 
 
 
static int Coe_Spline(COE_POINT *point)
{
    double *x = (*point).xCoordinate, *y = (*point).yCoordinate;
    int n = (*point).num - 1;
    coefficient *coe = (*point).coe;
    condition con = (*point).con;
    double *h, *d;
    double *a, *b, *c, *f, *m;
    double temp;
    int i;
    h = (double *)malloc( n * sizeof(double) ); /* 0,1--(n-1),n */
    d = (double *)malloc( n * sizeof(double) ); /* 0,1--(n-1),n */
     
    a = (double *)malloc( (n + 1) * sizeof(double) );  /* 特别使用,1--(n-1),       n */
    b = (double *)malloc( (n + 1) * sizeof(double) );  /*        0,1--(n-1),       n */
    c = (double *)malloc( (n + 1) * sizeof(double) );  /*        0,1--(n-1),特别使用 */
    f = (double *)malloc( (n + 1) * sizeof(double) );  /*        0,1--(n-1),       n */
    m = b;
    if(f == NULL)
    {
        free(h);
        free(d);
        free(a);
        free(b);
        free(c);
        return 1;
    }
    /* 计算 h[] d[] */
    for (i = 0; i < n; i++)
    {
        h[i] = x[i + 1] - x[i];
        d[i] = (y[i + 1] - y[i]) / h[i];
        /* printf("%f\t%f\n", h[i], d[i]); */
    }
    /**********************
    ** 初始化系数增广矩阵
    **********************/
    a[0] = (*point).f0;
    c[n] = (*point).fn;
    /* 计算 a[] b[] d[] f[] */
    switch(con)
    {
        case NATURAL:
            f[0] = a[0];
            f[n] = c[n];
            a[0] = 0;
            c[n] = 0;
            c[0] = 0;
            a[n] = 0;
            b[0] = 1;
            b[n] = 1;
        break;
         
        case CLAMPED:
            f[0] = 6 * (d[0] - a[0]);
            f[n] = 6 * (c[n] - d[n - 1]);
            a[0] = 0;
            c[n] = 0;
            c[0] = h[0];
            a[n] = h[n - 1];
            b[0] = 2 * h[0];
            b[n] = 2 * h[n - 1];
        break;
         
        case NOTaKNOT:
            f[0] = 0;
            f[n] = 0;
            a[0] = h[0];
            c[n] = h[n - 1];
            c[0] = -(h[0] + h[1]);
            a[n] = -(h[n - 2] + h[n - 1]);
            b[0] = h[1];
            b[n] = h[n - 2];
        break;
    }
 
    for (i = 1; i < n; i++)
    {
        a[i] = h[i - 1];
        b[i] = 2 * (h[i - 1] + h[i]);
        c[i] = h[i];
        f[i] = 6 * (d[i] - d[i - 1]);
    }
    /* for (i = 0; i < n+1; i++)   printf("%f\n", c[i]); */
     
    /*************
    ** 高斯消元
    *************/
    /* 第0行到第(n-3)行的消元 */
    for(i = 0; i <= n - 3; i++)
    {
        /* 选主元 */
        if( fabs(a[i + 1]) > fabs(b[i]) )
        {
            temp = a[i + 1]; a[i + 1] = b[i]; b[i] = temp;
            temp = b[i + 1]; b[i + 1] = c[i]; c[i] = temp;
            temp = c[i + 1]; c[i + 1] = a[i]; a[i] = temp;
            temp = f[i + 1]; f[i + 1] = f[i]; f[i] = temp;
        }
        temp = a[i + 1] / b[i];
        a[i + 1] = 0;
        b[i + 1] = b[i + 1] - temp * c[i];
        c[i + 1] = c[i + 1] - temp * a[i];
        f[i + 1] = f[i + 1] - temp * f[i];
    }
     
    /* 最后3行的消元 */
    /* 选主元 */
    if( fabs(a[n - 1]) > fabs(b[n - 2]) )
    {
        temp = a[n - 1]; a[n - 1] = b[n - 2]; b[n - 2] = temp;
        temp = b[n - 1]; b[n - 1] = c[n - 2]; c[n - 2] = temp;
        temp = c[n - 1]; c[n - 1] = a[n - 2]; a[n - 2] = temp;
        temp = f[n - 1]; f[n - 1] = f[n - 2]; f[n - 2] = temp;
    }
    /* 选主元 */
    if( fabs(c[n]) > fabs(b[n - 2]) )
    {
        temp = c[n]; c[n] = b[n - 2]; b[n - 2] = temp;
        temp = a[n]; a[n] = c[n - 2]; c[n - 2] = temp;
        temp = b[n]; b[n] = a[n - 2]; a[n - 2] = temp;
        temp = f[n]; f[n] = f[n - 2]; f[n - 2] = temp;
    }
    /* 第(n-1)行消元 */
    temp = a[n - 1] / b[n - 2];
    a[n - 1] = 0;
    b[n - 1] = b[n - 1] - temp * c[n - 2];
    c[n - 1] = c[n - 1] - temp * a[n - 2];
    f[n - 1] = f[n - 1] - temp * f[n - 2];
    /* 第n行消元 */
    temp = c[n] / b[n - 2];
    c[n] = 0;
    a[n] = a[n] - temp * c[n - 2];
    b[n] = b[n] - temp * a[n - 2];
    f[n] = f[n] - temp * f[n - 2];
    /* 选主元 */
    if( fabs(a[n]) > fabs(b[n - 1]) )
    {
        temp = a[n]; a[n] = b[n - 1]; b[n - 1] = temp;
        temp = b[n]; b[n] = c[n - 1]; c[n - 1] = temp;
        temp = f[n]; f[n] = f[n - 1]; f[n - 1] = temp;
    }
    /* 最后一次消元 */
    temp = a[n] / b[n-1];
    a[n] = 0;
    b[n] = b[n] - temp * c[n - 1];
    f[n] = f[n] - temp * f[n - 1];
     
    /* 回代求解 m[] */
    m[n] = f[n] / b[n];
    m[n - 1] = (f[n - 1] - c[n - 1] * m[n]) / b[n-1];
    for(i = n - 2; i >= 0; i--)
    {
        m[i] = ( f[i] - (m[i + 2] * a[i] + m[i + 1] * c[i]) ) / b[i];
    }
    /* for (i = 0; i < n+1; i++)   printf("%f\n", m[i]); */
    free(a);
    free(c);
    free(f);
    /************
    ** 放置参数
    ************/
    for(i = 0; i < n; i++)
    {
        coe[i].a3 = (m[i + 1] - m[i]) / (6 * h[i]);
        coe[i].b2 = m[i] / 2;
        coe[i].c1 = d[i] - (h[i] / 6) * (2 * m[i] + m[i + 1]);
        coe[i].d0 = y[i];
    }
     
    free(h);
    free(d);
    free(b);
    return n + 1;
}

static void GetCurvePoints_draw(vector<CPoint>* keypts, Point * Points)
{
	if (NULL == Points || NULL == keypts)
		return;
	
	double x[LINE_WINDOW_WIDTH] = {0};
	double y[LINE_WINDOW_WIDTH] = {0};
	
    int n = (*keypts).size();
    coefficient *coe;
    int i;
    COE_POINT p;
    coe = (coefficient *)malloc((n - 1) * sizeof(coefficient));
	
	for (i=0; i<n; i++)
	{
		x[i] = (*keypts)[i].x;
		y[i] = (*keypts)[i].y;
	}
	
    p.xCoordinate = x;
    p.yCoordinate = y;
    p.f0 = 0;
    p.fn = 0;
    p.num = n;
    p.con = NATURAL;
    p.coe = coe;
    Coe_Spline(&p);
	
	for (int j=0; j<x[0]; j++)
	{
		Points[j].X = j;
		Points[j].Y = y[0];
	}
	
	for (i=0; i<n; i++)
	{
		for (j=x[i]; j<x[i+1]; j++)
		{
			Points[j].X = j;
			Points[j].Y = coe[i].d0 + coe[i].c1 * (j - x[i]) 
				+ coe[i].b2 * (j - x[i]) * (j - x[i]) 
				+ coe[i].a3 * (j - x[i]) * (j - x[i]) * (j - x[i]);
		}
	}
	
	for (j=x[n-1]; j<LINE_WINDOW_WIDTH; j++)
	{
		Points[j].X = j;
		Points[j].Y = y[n-1];
	}
	
    free(coe);
}

static void GetCurvePoints(vector<CPoint>* keypts, Point * Points, int level_num)
{
	if (NULL == Points || NULL == keypts)
		return;

	double x[LINE_WINDOW_WIDTH] = {0};
	double y[LINE_WINDOW_WIDTH] = {0};
	double temp_x = 0, temp_y = 0;
    int n = (*keypts).size();
    coefficient *coe;
    int i;
    COE_POINT p;
    coe = (coefficient *)malloc((n - 1) * sizeof(coefficient));

	temp_x = (double)level_num / (double)LINE_WINDOW_WIDTH;
	temp_y = (double)(LINE_Y_MAX - LINE_Y_MIN)/(double)LINE_WINDOW_WIDTH;
	for (i=0; i<n; i++)
	{
		x[i] = (*keypts)[i].x*temp_x;
		y[i] = (*keypts)[i].y*temp_y;
	}

    p.xCoordinate = x;
    p.yCoordinate = y;
    p.f0 = 0;
    p.fn = 0;
    p.num = n;
    p.con = NATURAL;
    p.coe = coe;
    Coe_Spline(&p);

	for (int j=0; j<x[0]; j++)
	{
		Points[j].X = j;
		Points[j].Y = y[0];
	}

	for (i=0; i<n; i++)
	{
		for (j=x[i]; j<x[i+1]; j++)
		{
			Points[j].X = j;
			Points[j].Y = coe[i].d0 + coe[i].c1 * (j - x[i]) 
				+ coe[i].b2 * (j - x[i]) * (j - x[i]) 
				+ coe[i].a3 * (j - x[i]) * (j - x[i]) * (j - x[i]);
		}
	}

	for (j=x[n-1]; j<LINE_WINDOW_WIDTH; j++)
	{
		Points[j].X = j;
		Points[j].Y = y[n-1];
	}
	
    free(coe);
}


/////////////////////////////////////////////////////////////////////////////
// CLineDlg dialog


CLineDlg::CLineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLineDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_level_num = LINE_LEVEL_NUM;

	m_keyPts.clear();
	m_keyPts.push_back(CPoint(0, LINE_WINDOW_HEIGHT/2));
	m_keyPts.push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));
}


void CLineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLineDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLineDlg, CDialog)
	//{{AFX_MSG_MAP(CLineDlg)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLineDlg message handlers


BOOL CLineDlg::OnInitDialog()
{
	//装载gdi+
	GdiplusStartupInput m_gdiplusStartupInput;
    GdiplusStartup(&m_pGdiToken,&m_gdiplusStartupInput,NULL);

	m_drag = FALSE;
	ZeroMemory(m_level, sizeof(m_level));
	m_moveflag = -1;


	CDC* pDC = GetDC();

	m_MemDC.CreateCompatibleDC(pDC);
	
	m_MemBitmap.CreateCompatibleBitmap(pDC, 500, 500);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	ReleaseDC(pDC);

	this->GetClientRect(&m_Rect);

	m_CurveRect.SetRect(m_Rect.left + 50, m_Rect.top + 10, 
		m_Rect.left + 50 + LINE_WINDOW_WIDTH, m_Rect.top + 10 + LINE_WINDOW_HEIGHT);

	m_CurveFrameRect.left = m_CurveRect.left - 2;
	m_CurveFrameRect.right = m_CurveRect.right + 2;
	m_CurveFrameRect.top = m_CurveRect.top - 2;
	m_CurveFrameRect.bottom = m_CurveRect.bottom + 2;


	m_handCursor = ::LoadCursor(NULL, IDC_HAND);
	
	return TRUE;
}

void CLineDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
}


#define CURVE_LINE_COLOR		Color(0, 0, 0)		//BLACK
#define CONTROL_POINT_COLOR		Color(255, 0, 0)	//Red
#define BASE_LINE_COLOR			Color(150, 150, 150)

void CLineDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 __super::OnPaint()

	//draw frame
	dc.DrawEdge(m_CurveFrameRect, EDGE_BUMP, BF_RECT);
	
	m_MemDC.FillSolidRect(0, 0, m_CurveRect.Width(), m_CurveRect.Height(), RGB(189, 252, 201));
	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);

	CBrush brush(RGB(255, 0, 0));
	CBrush* pOldBrush;

	pOldBrush = m_MemDC.SelectObject(&brush);

	CPen pen(PS_SOLID, 4, RGB(255, 0, 0));
	CPen* pOldPen = m_MemDC.SelectObject(&pen);
	Graphics graphics(m_MemDC);

	//draw base line
	Pen gdiBaseLinePen(BASE_LINE_COLOR);
	graphics.DrawLine(&gdiBaseLinePen, Point(0, LINE_WINDOW_HEIGHT/2), Point(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));
	graphics.DrawLine(&gdiBaseLinePen, Point(LINE_WINDOW_WIDTH/2, 0), Point(LINE_WINDOW_WIDTH/2, LINE_WINDOW_HEIGHT));

	int steplen = LINE_WINDOW_WIDTH / 16;

	for (int i=1; i<16; i++)
	{
		graphics.DrawLine(&gdiBaseLinePen, Point(LINE_WINDOW_WIDTH/2-1, steplen*i), Point(LINE_WINDOW_WIDTH/2+1, steplen*i));
		graphics.DrawLine(&gdiBaseLinePen, Point(steplen*i, LINE_WINDOW_HEIGHT/2-1), Point(steplen*i, LINE_WINDOW_HEIGHT/2+1));
	}

	// draw curve
	Point * Points = NULL;
	Points = new Point[LINE_WINDOW_WIDTH];
	Pen gdiCurvePen(CURVE_LINE_COLOR);

	GetCurvePoints_draw(&m_keyPts, Points);

	graphics.DrawLines(&gdiCurvePen, Points, LINE_WINDOW_WIDTH);
	
	delete[] Points;
	Points = NULL;

	//draw control point

	SolidBrush gdiControlPosBrush(CONTROL_POINT_COLOR);
	vector<CPoint>::iterator iter2 = m_keyPts.begin();
	for (; iter2 != m_keyPts.end(); ++iter2) 
	{
		Point points[4] = {Point(iter2->x, iter2->y - 3), Point(iter2->x - 3, iter2->y), 
						   Point(iter2->x, iter2->y + 3), Point(iter2->x + 3, iter2->y)};
		graphics.FillPolygon(&gdiControlPosBrush, points, 4);
	}


	m_MemDC.SelectObject(pOldBrush);
	brush.DeleteObject();
	m_MemDC.SelectObject(pOldPen);
	pen.DeleteObject();
	m_MemDC.SetBkMode(oldBkMode);
	
	dc.BitBlt(m_CurveRect.left, m_CurveRect.top, m_CurveRect.Width(), m_CurveRect.Height(), &m_MemDC, 0, 0, SRCCOPY);

}

void CLineDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}

	//卸载gdi+
    GdiplusShutdown(m_pGdiToken);
	
	CDialog::OnClose();
}

void CLineDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_CurveRect.PtInRect(point)) {
		CDialog::OnMouseMove(nFlags, point);
		m_drag = FALSE;
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_CurveRect.left;
	InPoint.y = point.y - m_CurveRect.top;
	
	if (m_drag && m_moveflag > 0 && (unsigned int)(m_moveflag) < m_keyPts.size() - 1) {
		if (InPoint.x > m_keyPts[m_moveflag - 1].x + CONTROL_POINT_APART && 
			InPoint.x < m_keyPts[m_moveflag + 1].x - CONTROL_POINT_APART)
			m_keyPts[m_moveflag] = InPoint;
		else {
			vector<CPoint>::iterator iterRemove = m_keyPts.begin() + m_moveflag;
			m_keyPts.erase(iterRemove);
			m_moveflag = -1;
			m_drag = FALSE;
		}
	}

	if (m_drag && m_moveflag == 0 && InPoint.x < m_keyPts[1].x - CONTROL_POINT_APART)
		m_keyPts[0] = InPoint;

    if (m_drag && m_moveflag == m_keyPts.size() - 1 && 
		InPoint.x > m_keyPts[m_keyPts.size() - 2].x + CONTROL_POINT_APART)
        m_keyPts[m_moveflag] = InPoint;

	InvalidateRect(m_CurveRect, FALSE);
	
	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < m_keyPts.size()) {
		CString strCoord;
		CWnd * pCoordTextX = GetDlgItem(IDC_STATIC_X);
		CWnd * pCoordTextY = GetDlgItem(IDC_STATIC_Y);
		
		strCoord.Format("%d", InPoint.x * m_level_num / m_CurveRect.Width() + LINE_X_MIN);
		pCoordTextX->SetWindowText(strCoord);
		strCoord.Format("%d", LINE_Y_MAX - InPoint.y * (LINE_Y_MAX - LINE_Y_MIN) / m_CurveRect.Height());
		pCoordTextY->SetWindowText(strCoord);
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CLineDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_drag)
		m_drag = FALSE;

	CDialog::OnLButtonUp(nFlags, point);
}

void CLineDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_CurveRect.PtInRect(point)) {
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	GetDlgItem(IDC_BUTTON_RESET)->SetFocus();
	
	CPoint InPoint;
	InPoint.x = point.x - m_CurveRect.left;
	InPoint.y = point.y - m_CurveRect.top;

	for (unsigned int i = 1; i < m_keyPts.size(); ++i)
	{
		if (InPoint.x > m_keyPts[i - 1].x + CONTROL_POINT_APART && InPoint.y > 0 && 
			InPoint.x < m_keyPts[i].x - CONTROL_POINT_APART && InPoint.y < m_CurveRect.Height())
		{
			vector<CPoint>::iterator iterInsert = m_keyPts.begin() + i;
			m_keyPts.insert(iterInsert, InPoint);
			m_drag = TRUE;
			m_moveflag = i;
			::SetCursor(m_handCursor);
			InvalidateRect(m_CurveRect, FALSE);
		}
	}
	
	vector<CPoint>::iterator iter = m_keyPts.begin();
	CRect rect(InPoint.x - 3, InPoint.y - 3, InPoint.x + 3, InPoint.y + 3);
	for (i = 0; iter != m_keyPts.end(); ++iter, ++i) {
		if (rect.PtInRect(*iter)) {
			m_drag = TRUE;
			m_moveflag = i;
		}
	}
	
	CString strCoord;
	CWnd * pCoordTextX = GetDlgItem(IDC_STATIC_X);
	CWnd * pCoordTextY = GetDlgItem(IDC_STATIC_Y);
	
	strCoord.Format("%d", InPoint.x * m_level_num / m_CurveRect.Width() + LINE_X_MIN);
		pCoordTextX->SetWindowText(strCoord);
		strCoord.Format("%d", LINE_Y_MAX - InPoint.y * (LINE_Y_MAX - LINE_Y_MIN) / m_CurveRect.Height());
		pCoordTextY->SetWindowText(strCoord);

	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CLineDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_drag) {
		::SetCursor(m_handCursor);
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CLineDlg::GetImageLevel(void)
{
	Point * Points = NULL;
	int i = 0;
	vector<Point> pts;
	Points = new Point[LINE_WINDOW_WIDTH];
	pts.resize(LINE_WINDOW_WIDTH);

	GetCurvePoints(&m_keyPts, Points, m_level_num);

	for (i = 0; i < pts.size(); ++i) {
		//pts[i].X = Points[i].X * m_level_num / LINE_WINDOW_WIDTH;
		//pts[i].Y = LINE_Y_MAX - Points[i].Y * (LINE_Y_MAX - LINE_Y_MIN) / LINE_WINDOW_HEIGHT;
		pts[i].X = Points[i].X;
		pts[i].Y = LINE_Y_MAX - Points[i].Y;

	}

	for (i = 0; i < m_level_num; ++i)
	{
		//int value = pts[i*LINE_WINDOW_WIDTH/m_level_num].Y;
		int value = pts[i].Y;
		if (value < LINE_Y_MIN) value = LINE_Y_MIN;
		if (value > LINE_Y_MAX) value = LINE_Y_MAX;
		m_level[pts[0].X + i] = (short)value;
	}

	m_level[m_level_num-1] = pts[LINE_WINDOW_WIDTH-1].Y;

	delete[] Points;
	Points = NULL;
}

void CLineDlg::GetLevel(char* pbuf, int* size)
{
	if (NULL == pbuf || *size < m_level_num * sizeof(unsigned short))
	{
		return;
	}

	GetImageLevel();

	memcpy(pbuf, m_level, m_level_num * sizeof(unsigned short));
	*size = m_level_num * sizeof(unsigned short);
}


void CLineDlg::SetLevel(char* pbuf, int size)
{
	if (NULL == pbuf || size < m_level_num * sizeof(unsigned short))
	{
		return;
	}

	memcpy(m_level, pbuf, m_level_num * sizeof(unsigned short));
}

BOOL CLineDlg::PreTranslateMessage(MSG * pMsg)
{
	// TODO: 在此添加控件通知处理程序代码
	if (pMsg->message == WM_KEYDOWN) {
		SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CLineDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_moveflag < 0 || m_moveflag >= m_keyPts.size()) {
		CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}
	
	CPoint point = m_keyPts[m_moveflag];
	BOOL bMove = FALSE;

	switch(nChar) {
	case VK_LEFT:
		point.x -= 1;
		bMove = TRUE;
		break;
	case VK_RIGHT:
		point.x += 1;
		bMove = TRUE;
		break;
	case VK_UP:
		point.y -= 1;
		bMove = TRUE;
		break;
	case VK_DOWN:
		point.y += 1;
		bMove = TRUE;
		break;
	default:
		bMove = FALSE;
		break;
	}
	
	if (bMove) {
		BOOL bTempDrag = m_drag;
		m_drag = TRUE;

		point.x += m_CurveRect.left;
		point.y += m_CurveRect.top;

		OnMouseMove(nFlags, point);
		m_drag = bTempDrag;
	}

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

vector<CPoint>* CLineDlg::GetKeyPts(void)
{
	return &m_keyPts;
}


void CLineDlg::Refresh(void)
{
	InvalidateRect(m_CurveRect, FALSE);
}


void CLineDlg::OnButtonReset() 
{
	// TODO: Add your control notification handler code here
	ZeroMemory(m_level, sizeof(m_level));

	m_drag = FALSE;
	m_moveflag = -1;

	m_keyPts.clear();
	m_keyPts.push_back(CPoint(0, LINE_WINDOW_HEIGHT/2));
	m_keyPts.push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));

	InvalidateRect(m_CurveRect, FALSE);
}


void CLineDlg::SetKeyPts(vector<CPoint>* keypts, T_U16 *key, T_S16* curve)
{
	int cnt = 0;

	(*keypts).clear();

	for (int i=0; i<LINE_KEY_POINT_MAX; i++)
	{
		if ((0 != key[i]))
		{
			cnt++;
		}
	}

	if (0 == cnt || cnt > LINE_KEY_POINT_MAX)
	{
		int step = m_level_num / LINE_KEY_POINT_MAX;

		for (i=0; i<LINE_KEY_POINT_MAX; i++)
		{
			(*keypts).push_back(CPoint(step * i * LINE_WINDOW_WIDTH / m_level_num, (LINE_Y_MAX+1 - curve[step * i]) * LINE_WINDOW_HEIGHT / (LINE_Y_MAX - LINE_Y_MIN + 1)));
		}

		(*keypts).push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));
	}
	else 
	{
		bool flag = 0;

		for (i=0; i<LINE_KEY_POINT_MAX; i++)
		{
			if ((0 == key[i]) && (i>0))
			{
				break;
			}

			if (key[i] <= m_level_num-1)
			{				
				if (m_level_num-1 == key[i])
				{
					flag = 1;
					(*keypts).push_back(CPoint(LINE_WINDOW_WIDTH-1, (LINE_Y_MAX+1 - curve[key[i]]) * LINE_WINDOW_HEIGHT / (LINE_Y_MAX - LINE_Y_MIN + 1)));
				}
				else
				{
					(*keypts).push_back(CPoint(key[i] * LINE_WINDOW_WIDTH / m_level_num, (LINE_Y_MAX+1 - curve[key[i]]) * LINE_WINDOW_HEIGHT / (LINE_Y_MAX - LINE_Y_MIN + 1)));			
				}
			}
			
		}

		if (!flag)
		{
			(*keypts).push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));
		}
		
	}
}