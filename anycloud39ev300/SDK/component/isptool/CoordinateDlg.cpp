// CoordinateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "CoordinateDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Gdiplus;

#define PI	3.1415926
#define CONTROL_POINT_APART		(COORDINATE_WINDOW_WIDTH / m_level_num)

#define GBR_64	64


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

static void GetCurvePoints(vector<CPoint>* keypts, Point * Points)
{
	if (NULL == Points || NULL == keypts)
		return;

	double x[COORDINATE_WINDOW_HEIGHT] = {0};
	double y[COORDINATE_WINDOW_HEIGHT] = {0};

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

	for (j=x[n-1]; j<COORDINATE_WINDOW_HEIGHT; j++)
	{
		Points[j].X = j;
		Points[j].Y = y[n-1];
	}
	
    free(coe);
}

/////////////////////////////////////////////////////////////////////////////
// CCoordinateDlg dialog


CCoordinateDlg::CCoordinateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCoordinateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCoordinateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	//m_keyPts.clear();
	//m_keyPts.push_back(CPoint(0, COORDINATE_WINDOW_WIDTH));
	//m_keyPts.push_back(CPoint(0, COORDINATE_WINDOW_WIDTH));

	m_update_flag = FALSE;
	m_calc_flag = FALSE;
}


void CCoordinateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCoordinateDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCoordinateDlg, CDialog)
	//{{AFX_MSG_MAP(CCoordinateDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_COORDINATE_RESET, OnButtonCoordinateReset)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_GET_WB_INFO, OnButtonGetWbInfo)
	ON_BN_CLICKED(IDC_BUTTON_SET_WB_INFO, OnButtonSetWbInfo)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_CALC_WB, OnButtonCalcWb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoordinateDlg message handlers
void CCoordinateDlg::Init_keyPoint_reset() 
{
	UINT i = 0, j = 0, idex = 0;
	long temp = 200;
	float temp_x = 0;
	float temp_y = 0;
	

	for (i = 0; i < 10; i++)
	{
		for (j = 0; j < 2; j++)
		{
			m_keyPoint_GBR[i][j].x = 0;
			m_keyPoint_GBR[i][j].y = 0;
			m_keyPoint_BR[i][j].x = 0;
			temp_y = 100;

			if (j == 0 && m_coor_Isp_wb.p_awb.rb_low[i] != 0)
			{
				temp_y = (COORDINATE_WINDOW_WIDTH - temp_y);//为了下面的X值
				temp_x = (float)temp_y/(float)(m_coor_Isp_wb.p_awb.rb_low[i]/(float)GBR_64);
				if (temp_x > 511)
				{
					temp_x = 500;//(定的范围是0~255)
					temp_y = temp_x*(m_coor_Isp_wb.p_awb.rb_low[i]/(float)GBR_64);
				}
			}
			else if (j == 1 && m_coor_Isp_wb.p_awb.rb_high[i] != 0)
			{
				temp_y = (COORDINATE_WINDOW_WIDTH - temp_y);
				temp_x = (float)(temp_y)/((float)m_coor_Isp_wb.p_awb.rb_high[i]/(float)GBR_64);
				if (temp_x > 511)
				{
					temp_x = 500;//(定的范围是0~255)
					temp_y = temp_x*((float)m_coor_Isp_wb.p_awb.rb_high[i]/(float)GBR_64);
				}
			}
			temp = COORDINATE_WINDOW_WIDTH - temp_y;

			m_keyPoint_BR[i][j].x = temp_x;
			m_keyPoint_BR[i][j].y = temp;

			
		}
	}

}

void CCoordinateDlg::Get_keyPoint() 
{
	UINT i = 0, j = 0;
	float temp = 0, temp1 = 0, temp2 = 0;
	
	for (i = 0; i < 10; i++)
	{
		m_coor_Isp_wb.p_awb.gr_low[i] = m_keyPoint_GBR[i][0].x/GBR_MUTIL; //A //lt84 //d50 //d65 //d75    
		m_coor_Isp_wb.p_awb.gb_high[i] = (COORDINATE_WINDOW_WIDTH - m_keyPoint_GBR[i][0].y)/GBR_MUTIL;       
		m_coor_Isp_wb.p_awb.gr_high[i] = m_keyPoint_GBR[i][1].x/GBR_MUTIL;       
		m_coor_Isp_wb.p_awb.gb_low[i] = (COORDINATE_WINDOW_WIDTH - m_keyPoint_GBR[i][1].y)/GBR_MUTIL;

		m_coor_Isp_wb.p_awb.rb_low[i] = (COORDINATE_WINDOW_WIDTH - (float)m_keyPoint_BR[i][0].y)/(float)m_keyPoint_BR[i][0].x*(float)GBR_64;
		m_coor_Isp_wb.p_awb.rb_high[i] = (COORDINATE_WINDOW_WIDTH - (float)m_keyPoint_BR[i][1].y)/(float)m_keyPoint_BR[i][1].x*(float)GBR_64;
	}

}

void CCoordinateDlg::Set_keyPoint(AK_ISP_AWB_ATTR *p_awb) 
{
	UINT i = 0, j = 0;
	float temp = 0, temp1 = 0, temp2 = 0;

	for (i = 0; i < 10; i++)
	{
		m_keyPoint_GBR[i][0].x = p_awb->gr_low[i]*GBR_MUTIL; //A //lt84 //d50 //d65 //d75    
		m_keyPoint_GBR[i][0].y = COORDINATE_WINDOW_WIDTH - p_awb->gb_high[i]*GBR_MUTIL;       
		m_keyPoint_GBR[i][1].x = p_awb->gr_high[i]*GBR_MUTIL;       
		m_keyPoint_GBR[i][1].y = COORDINATE_WINDOW_WIDTH - p_awb->gb_low[i]*GBR_MUTIL;

		temp2 = (float)p_awb->rb_low[i]/(float)GBR_64;
		temp = m_keyPoint_BR[i][0].x *temp2;

		if (temp > 511)
		{
			temp = 500;
			m_keyPoint_BR[i][0].x = temp/temp2;
		}
		m_keyPoint_BR[i][0].y  = (COORDINATE_WINDOW_WIDTH - temp);


		temp2 = (float)p_awb->rb_high[i]/(float)GBR_64;
		temp = m_keyPoint_BR[i][1].x *temp2;

		if (temp > 511)
		{
			temp = 500;
			m_keyPoint_BR[i][1].x = temp/temp2;
		}
		m_keyPoint_BR[i][1].y  = (COORDINATE_WINDOW_WIDTH - temp);
	}

}


BOOL CCoordinateDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	//装载gdi+
	GdiplusStartupInput m_gdiplusStartupInput;
    GdiplusStartup(&m_pGdiToken,&m_gdiplusStartupInput,NULL);
	
	m_drag = FALSE;
	m_drag_br = FALSE;
	ZeroMemory(m_level, sizeof(m_level));
	m_moveflag = -1;
	m_moveflag_br = -1;

	Init_keyPoint_reset();
	Set_keyPoint(&m_coor_Isp_wb.p_awb);

	StartTimer();
	
	CDC* pDC = GetDC();
	
	m_MemDC.CreateCompatibleDC(pDC);
	
	m_MemBitmap.CreateCompatibleBitmap(pDC, COORDINATE_WINDOW_WIDTH, COORDINATE_WINDOW_HEIGHT);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	ReleaseDC(pDC);
	
	this->GetClientRect(&m_Rect);
	
	m_CoorRect.SetRect(m_Rect.left + 50, m_Rect.top + 10, 
		m_Rect.left + 50 + COORDINATE_WINDOW_WIDTH, m_Rect.top + 10 + COORDINATE_WINDOW_HEIGHT);
	
	m_CoorFrameRect.left = m_CoorRect.left - 2;
	m_CoorFrameRect.right = m_CoorRect.right + 2;
	m_CoorFrameRect.top = m_CoorRect.top - 2;
	m_CoorFrameRect.bottom = m_CoorRect.bottom + 2;
	
	m_handCursor = ::LoadCursor(NULL, IDC_HAND);


	InvalidateRect(m_CoorRect, FALSE);
	UpdateData(TRUE);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#define CURVE_LINE_COLOR		Color(0, 0, 0)		//BLACK
#define CONTROL_POINT_COLOR		Color(255, 0, 0)	//Red
#define BASE_LINE_COLOR			Color(150, 150, 150)

void CCoordinateDlg::OnPaint() 
{
	UINT i = 0,j = 0;
	T_U16 rect_left = m_CoorRect.left;
	T_U16 rect_top = m_CoorRect.top;

	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 __super::OnPaint()
	
	//draw frame
	dc.DrawEdge(m_CoorFrameRect, EDGE_BUMP, BF_RECT);

	m_MemDC.FillSolidRect(0, 0, m_CoorRect.Width(), m_CoorRect.Height(), RGB(189, 252, 201));
	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);
	
	CBrush brush(RGB(255, 0, 0));
	CBrush* pOldBrush;
	
	pOldBrush = m_MemDC.SelectObject(&brush);
	
	CPen pen(PS_SOLID, 4, RGB(255, 0, 0));
	CPen* pOldPen = m_MemDC.SelectObject(&pen);
	Graphics graphics(m_MemDC);
	
	//draw base line
	Pen gdiBaseLinePen(CURVE_LINE_COLOR);
	graphics.DrawLine(&gdiBaseLinePen, Point(0, 0), Point(0, COORDINATE_WINDOW_WIDTH-1));
	graphics.DrawLine(&gdiBaseLinePen, Point(0, COORDINATE_WINDOW_WIDTH-1), Point(COORDINATE_WINDOW_WIDTH-1, COORDINATE_WINDOW_WIDTH-1));

	for (i=1; i<16; i++)
	{
		graphics.DrawLine(&gdiBaseLinePen, Point(0, 32*i), Point(2, 32*i));
		graphics.DrawLine(&gdiBaseLinePen, Point(32*i, COORDINATE_WINDOW_WIDTH-1), Point(32*i, COORDINATE_WINDOW_WIDTH-2));
	}

	graphics.DrawLine(&gdiBaseLinePen, Point(0, 0), Point(-4, 4));
	graphics.DrawLine(&gdiBaseLinePen, Point(0, 0), Point(4, 4));

	graphics.DrawLine(&gdiBaseLinePen, Point(COORDINATE_WINDOW_WIDTH-1, COORDINATE_WINDOW_WIDTH-1), Point(COORDINATE_WINDOW_WIDTH-4, COORDINATE_WINDOW_WIDTH-4));
	graphics.DrawLine(&gdiBaseLinePen, Point(COORDINATE_WINDOW_WIDTH-1, COORDINATE_WINDOW_WIDTH-1), Point(COORDINATE_WINDOW_WIDTH+4, COORDINATE_WINDOW_WIDTH+4));
	
	//draw control point
	SolidBrush gdiControlPosBrush(CURVE_LINE_COLOR);
	//draw keypoints
	for (i = 0; i < InPoint_GBR_num; i++) 
	{
		Point points[4] = 
			{
			Point(InPoint_GBR[i].x, (InPoint_GBR[i].y - 1)), 
			Point((InPoint_GBR[i].x - 1), InPoint_GBR[i].y), 
			Point(InPoint_GBR[i].x, (InPoint_GBR[i].y + 1)), 
			Point((InPoint_GBR[i].x + 1), InPoint_GBR[i].y)
			};
		graphics.FillPolygon(&gdiControlPosBrush, points, 4);
	}

	m_MemDC.SelectObject(pOldBrush);
	brush.DeleteObject();
	m_MemDC.SelectObject(pOldPen);
	pen.DeleteObject();
	m_MemDC.SetBkMode(oldBkMode);
	
	dc.BitBlt(m_CoorRect.left, m_CoorRect.top, m_CoorRect.Width(), m_CoorRect.Height(), &m_MemDC, 0, 0, SRCCOPY);

	//draw select rect line
	CPen pen_red(PS_SOLID, 1, RGB(255, 0, 0));
	CPen pen_orange(PS_SOLID, 1, RGB(255, 128, 0));
	CPen pen_yellow(PS_SOLID, 1, RGB(255, 255, 0));
	CPen pen_green(PS_SOLID, 1, RGB(0, 255, 0));
	CPen pen_blue(PS_SOLID, 1, RGB(0, 0, 255));
	CPen pen_black(PS_SOLID, 1, RGB(0, 0, 0));

	CPen pen_default1(PS_SOLID, 1, RGB(255, 128, 200)); //粉色
	CPen pen_default2(PS_SOLID, 1, RGB(150, 0, 150)); //紫色
	CPen pen_default3(PS_SOLID, 1, RGB(0, 64, 0));//深绿
	CPen pen_default4(PS_SOLID, 1, RGB(128, 128, 0));//深黄色
	CPen pen_default5(PS_SOLID, 1, RGB(0, 200, 200));//天蓝色

	for (i = 0; i<10;i++)
	{
		if ( i == 0)
		{
			dc.SelectObject(&pen_red);
			if (!A_enable)
			{
				continue;
			}
		}
		else if(i == 1)
		{
			dc.SelectObject(&pen_orange);
			if (!TL84_enable)
			{
				continue;
			}
		}
		else if(i == 2)
		{
			dc.SelectObject(&pen_yellow);	
			if (!D50_enable)
			{
				continue;
			}
		}
		else if(i == 3)
		{
			dc.SelectObject(&pen_green);
			if (!D65_enable)
			{
				continue;
			}
		}
		else if(i == 4)
		{
			dc.SelectObject(&pen_blue);	
			if (!D75_enable)
			{
				continue;
			}
		}
		else if(i == 5)
		{
			dc.SelectObject(&pen_default1);	
			if (!Default1_enable)
			{
				continue;
			}
		}
		else if(i == 6)
		{
			dc.SelectObject(&pen_default2);	
			if (!Default2_enable)
			{
				continue;
			}
		}
		else if(i == 7)
		{
			dc.SelectObject(&pen_default3);	
			if (!Default3_enable)
			{
				continue;
			}
		}
		else if(i == 8)
		{
			dc.SelectObject(&pen_default4);	
			if (!Default4_enable)
			{
				continue;
			}
		}
		else if(i == 9)
		{
			dc.SelectObject(&pen_default5);	
			if (!Default5_enable)
			{
				continue;
			}
		}

		for (j=0; j<2; j++) 
		{
			if (j == 0)
			{
				dc.FillSolidRect(m_keyPoint_GBR[i][j].x+rect_left-2, m_keyPoint_GBR[i][j].y+rect_top-2, 5, 5, RGB(0, 0, 0));
			}
			else
			{
				dc.FillSolidRect(m_keyPoint_GBR[i][j].x+rect_left-2, m_keyPoint_GBR[i][j].y+rect_top-2, 5, 5, RGB(0, 0, 255));
			}
		}

		dc.MoveTo(m_keyPoint_GBR[i][0].x+rect_left, m_keyPoint_GBR[i][0].y+rect_top);
		dc.LineTo(m_keyPoint_GBR[i][1].x+rect_left, m_keyPoint_GBR[i][0].y+rect_top);
		dc.LineTo(m_keyPoint_GBR[i][1].x+rect_left, m_keyPoint_GBR[i][1].y+rect_top);
		dc.LineTo(m_keyPoint_GBR[i][0].x+rect_left, m_keyPoint_GBR[i][1].y+rect_top);
		dc.LineTo(m_keyPoint_GBR[i][0].x+rect_left, m_keyPoint_GBR[i][0].y+rect_top);


		
		for (j=0; j<2; j++) 
		{
			if (j == 0)
			{
				dc.FillSolidRect(m_keyPoint_BR[i][j].x+rect_left-2, m_keyPoint_BR[i][j].y+rect_top-2, 5, 5, RGB(0, 0, 255));
			}
			else
			{
				dc.FillSolidRect(m_keyPoint_BR[i][j].x+rect_left-2, m_keyPoint_BR[i][j].y+rect_top-2, 5, 5, RGB(0, 0, 0));
			}
		}
		//画RB线

		dc.MoveTo(0+rect_left, COORDINATE_WINDOW_WIDTH-1+rect_top);
		dc.LineTo(m_keyPoint_BR[i][0].x+rect_left, m_keyPoint_BR[i][0].y+rect_top);
		
		dc.MoveTo(0+rect_left, COORDINATE_WINDOW_WIDTH-1+rect_top);
		dc.LineTo(m_keyPoint_BR[i][1].x+rect_left, m_keyPoint_BR[i][1].y+rect_top);
	}
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}


void CCoordinateDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCoordinateDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	BOOL falg = FALSE;
	// TODO: Add your message handler code here and/or call default
	if (!m_CoorRect.PtInRect(point)) {
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}
	
	CPoint InPoint;
	InPoint.x = point.x - m_CoorRect.left;
	InPoint.y = point.y - m_CoorRect.top;
	
	CRect rect(InPoint.x - 3, InPoint.y - 3, InPoint.x + 3, InPoint.y + 3);
	for (int j = 0; j < 10; j++)
	{
		for (int i = 0; i < 2; i++) 
		{
			if (rect.PtInRect(m_keyPoint_GBR[j][i])) 
			{
				m_drag = TRUE;
				m_moveflag = i;
				m_move_idex_flag = j;
				falg = TRUE;
				break;
			}
			if (rect.PtInRect(m_keyPoint_BR[j][i])) 
			{
				m_drag_br = TRUE;
				m_moveflag_br = i;
				m_move_idex_flag_br = j;
				falg = TRUE;
				break;
			}
		}
		if (falg)
		{
			break;
		}
	}
	

	CDialog::OnLButtonDown(nFlags, point);
}

void CCoordinateDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (m_drag)
		m_drag = FALSE;

	if (m_drag_br)
		m_drag_br = FALSE;

	m_update_flag = TRUE;

	CDialog::OnLButtonUp(nFlags, point);
}

void CCoordinateDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (!m_CoorRect.PtInRect(point)) {
		CDialog::OnMouseMove(nFlags, point);
		m_drag = FALSE;
		return;
	}
	
	CPoint InPoint;
	InPoint.x = point.x - m_CoorRect.left;
	InPoint.y = point.y - m_CoorRect.top;
	
	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < 2
		&& m_move_idex_flag >= 0 && (unsigned int)(m_move_idex_flag) < 10) 
	{
		m_keyPoint_GBR[m_move_idex_flag][m_moveflag] = InPoint;
		InvalidateRect(m_CoorRect, FALSE);
	}
	if (m_drag_br && m_moveflag_br >= 0 && (unsigned int)(m_moveflag_br) < 2
		&& m_move_idex_flag_br >= 0 && (unsigned int)(m_move_idex_flag_br) < 10) 
	{
		m_keyPoint_BR[m_move_idex_flag_br][m_moveflag_br] = InPoint;
		InvalidateRect(m_CoorRect, FALSE);
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CCoordinateDlg::CoordinateReset()
{
	//Init_keyPoint_reset();
	OnButtonCoordinateReset();
}

void CCoordinateDlg::OnButtonCoordinateReset() 
{
	// TODO: Add your control notification handler code here
	m_keyPoint[0].x = m_keyPoint[0].y = 0;
	m_keyPoint[1].x = m_keyPoint[1].y = 0;

	InPoint_GBR_num = 0;
	if (InPoint_GBR != NULL)
	{
		free(InPoint_GBR);
		InPoint_GBR = NULL;
	}

	//Init_keyPoint_reset();

	Invalidate();
}

void CCoordinateDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if (m_MemDC.m_hDC)
	{
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}
	
	//卸载gdi+
    GdiplusShutdown(m_pGdiToken);

	InPoint_GBR_num = 0;
	if (InPoint_GBR != NULL)
	{
		free(InPoint_GBR);
		InPoint_GBR = NULL;
	}

	CDialog::OnClose();
	DestroyWindow();
}

void CCoordinateDlg::Close() 
{
	OnClose();
}

void CCoordinateDlg::OnButtonGetWbInfo() 
{
	UINT time1 = 0; 
	UINT time2 = 0;
	
	GetDlgItem(IDC_BUTTON_GET_WB_INFO)->EnableWindow(FALSE);//可用

	m_get_wbinfo_flag = 1;
	//OnPaint();
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) 
	{
		AfxMessageBox("m_pMessageWnd is null", MB_OK);
		GetDlgItem(IDC_BUTTON_GET_WB_INFO)->EnableWindow(TRUE);//可用
		return;
	}
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_WB_INFO, DIALOG_STAT, 0);
	
	time1 = GetTickCount();
	while (1)
	{
		time2 = GetTickCount();
		if (time2 - time1 > 1000)
		{
			GetDlgItem(IDC_BUTTON_GET_WB_INFO)->EnableWindow(TRUE);//可用
			AfxMessageBox("获取白平衡参数超时(1s)", MB_OK);
			return;
		}
		if (m_get_wbinfo_flag == 1)
		{
			break;
		}

		if (m_get_wbinfo_flag == -1)
		{
			m_get_wbinfo_flag = 0;
			GetDlgItem(IDC_BUTTON_GET_WB_INFO)->EnableWindow(TRUE);//可用
			return;
		}
		Sleep(200);
	}
	
	Set_keyPoint(&m_coor_Isp_wb.p_awb);
	GetDlgItem(IDC_BUTTON_GET_WB_INFO)->EnableWindow(TRUE);//可用
	AfxMessageBox("成功获取白平衡参数", MB_OK);
	OnPaint();
	Invalidate();
}

void CCoordinateDlg::ShowCalcWbInfo() 
{	
	Set_keyPoint(&m_calc_awb);
	OnPaint();
	Invalidate();
}

void CCoordinateDlg::OnButtonSetWbInfo() 
{
	// TODO: Add your control notification handler code here
	Get_keyPoint();

	if (NULL == m_pMessageWnd)
	{
		AfxMessageBox("m_pMessageWnd is null", MB_OK);
		return;
	}
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SET_WB_INFO, DIALOG_STAT, 0);

	AfxMessageBox("已成功更新白平衡参数设到白平衡模块上", MB_OK);
}

void CCoordinateDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_get_wbinfo_flag == 1)
	{
		UpdateData(TRUE);
		m_get_wbinfo_flag = 0;
		//Set_keyPoint();
	}
	CDialog::OnTimer(nIDEvent);
}

void  CCoordinateDlg::StartTimer()
{
    used_time = 0;
    ctrl_time = SetTimer(1, 1000, NULL);//设置时间
}

void  CCoordinateDlg::StopTimer()
{
    KillTimer(ctrl_time);//清空时间
    ctrl_time = 0;
}

void  CCoordinateDlg::inset_key()
{

	unsigned int i = 0;

	for (i = 0; i < InPoint_GBR_num; ++i)
	{
		vector<CPoint>::iterator iterInsert = m_keyPts.begin() + i;
		m_keyPts.insert(iterInsert, InPoint_GBR[i]);
	}
}

void CCoordinateDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}

void CCoordinateDlg::OnButtonCalcWb() 
{
	// TODO: Add your control notification handler code here
	m_calc_flag = TRUE;
}
