// HUE_lineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "HUE_lineDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHUE_lineDlg dialog

#define BASE_LINE_COLOR1   Color(64, 0, 128)
#define BASE_LINE_COLOR2   Color(180, 41, 214)
#define BASE_LINE_COLOR3   Color(255, 0, 255)
#define BASE_LINE_COLOR4   Color(128, 0, 255)
#define BASE_LINE_COLOR5   Color(184, 72, 103)
#define BASE_LINE_COLOR6   Color(240, 15, 60)
#define BASE_LINE_COLOR7   Color(255, 0, 0)
#define BASE_LINE_COLOR8   Color(255, 128, 64)
#define BASE_LINE_COLOR9   Color(0, 128, 64)
#define BASE_LINE_COLOR10  Color(79, 190, 35)
#define BASE_LINE_COLOR11  Color(0, 249, 0)
#define BASE_LINE_COLOR12  Color(128, 255, 128)
#define BASE_LINE_COLOR13  Color(0, 128, 128)
#define BASE_LINE_COLOR14  Color(49, 128, 132)
#define BASE_LINE_COLOR15  Color(0, 0, 255)
#define BASE_LINE_COLOR16  Color(0, 0, 181)


T_S16 init_ket_point_x[17] = {64, 48, 32, 16, 0, -16, -32, -48, -64, -48, -32, -16, 0, 16, 32, 48, 64};
T_S16 init_ket_point_y[17] = {0, 16, 32, 48, 64, 48, 32, 16, 0, -16, -32, -48, -64, -48, -32, -16, 0};

using namespace Gdiplus;

#define PI	3.1415926
#define CONTROL_POINT_APART		(HUE_LINE_WINDOW_WIDTH / m_level_num)


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

CHUE_lineDlg::CHUE_lineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHUE_lineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHUE_lineDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_level_num = LINE_LEVEL_NUM;
	
	m_keyPts.clear();
	m_keyPts.push_back(CPoint(0, HUE_LINE_WINDOW_WIDTH/2));
	m_keyPts.push_back(CPoint(HUE_LINE_WINDOW_WIDTH-1, HUE_LINE_WINDOW_WIDTH/2));
}


void CHUE_lineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHUE_lineDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHUE_lineDlg, CDialog)
	//{{AFX_MSG_MAP(CHUE_lineDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHUE_lineDlg message handlers

void CHUE_lineDlg::Set_init_DataValue() 
{
	UINT i = 0; 
	long double degree = 22.5;
    long double radian = 0;
	long double temp = 0;

	for (i = 0; i < KEY_POINT_NUM; i++)
	{
		radian = (degree*i) * PI / 180;
		temp = cos(radian);
		ket_point_x[i] = temp*64;
		temp = sin(radian);
		ket_point_y[i] = temp*64;
	}

}

void CHUE_lineDlg::SetDataValue() 
{
	UINT i = 0, idex = 0;
	double degree = 22.5;
	double degree_temp = 0;
	double radian = 0; 
	double temp_a = 0;

	for (i = 0; i < KEY_POINT_NUM; i ++)
	{
		temp_a = ((long double)m_hue_info.hue_lut_b[idex]/127);
		degree_temp = 0 - asin(temp_a)*180/PI;//角度
		//算出此点的x,y
		
		degree_temp = degree_temp + degree*i;
		radian = degree_temp * PI / 180;//弧度

		if (cos(radian) >= 0)
			ket_point_x[i] = cos(radian)*m_hue_info.hue_lut_s[idex] + 0.5;
		else
			ket_point_x[i] = cos(radian)*m_hue_info.hue_lut_s[idex] - 0.5;

		if (sin(radian) >= 0)
			ket_point_y[i] = sin(radian)*m_hue_info.hue_lut_s[idex] + 0.5;
		else
			ket_point_y[i] = sin(radian)*m_hue_info.hue_lut_s[idex] - 0.5;
		
		idex = idex + 4;
	}
}

void CHUE_lineDlg::GetDataValue() 
{
	UINT i = 0, idex = 0, idex_ch = 1, j = 0;
	long double degree = 22.5;
	long double degree_ch = 5.625;
	long double degree_temp = 0;
	long double degree_temp_last = 0;
	long double degree_temp_ch = 0;
	long double degree_temp_change_ch = 0;
	long double radian = 0; 
	long double radian_ch = 0; 
	long double temp_a = 0;
	long double temp_s = 0;
	long double temp_s_1 = 0;
	long double temp_s_2 = 0;
	long double temp_s_3 = 0;

	for (i = 0; i < KEY_POINT_NUM; i++)
	{
		if (ket_point_y[i] != 0 && ket_point_x[i] != 0)
		{

			if (ket_point_y[i] > 0 && ket_point_x[i] > 0)
			{
				temp_a = (long double)ket_point_y[i]/(long double)ket_point_x[i];
				degree_temp = atan(temp_a)*180/PI;   //角度
				
			}
			else if(ket_point_y[i] > 0 && ket_point_x[i] < 0)
			{
				temp_a = (long double)ket_point_x[i]/(long double)ket_point_y[i];
				degree_temp = (-1)*atan(temp_a)*180/PI;   //角度
				degree_temp = degree_temp + 90;
			}
			else if(ket_point_y[i] < 0 && ket_point_x[i] < 0)
			{
				temp_a = (long double)ket_point_y[i]/(long double)ket_point_x[i];
				degree_temp = atan(temp_a)*180/PI;   //角度
				degree_temp = degree_temp + 180;
			}
			else
			{
				temp_a = (long double)ket_point_x[i]/(long double)ket_point_y[i];
				degree_temp = (-1)*atan(temp_a)*180/PI;   //角度
				degree_temp = degree_temp + 270;	
			}
			
			radian = degree_temp * PI / 180;//弧度
			m_hue_info.hue_lut_s[i*4] = ket_point_x[i]/cos(radian) + 0.5;

			//degree_temp = degree_temp - degree*i;
			degree_temp = degree*i-degree_temp ;

			//记录每一个角度
			ket_point_degree[i] = degree_temp;

			radian = degree_temp * PI / 180;//弧度

			m_hue_info.hue_lut_a[i*4] = cos(radian)*128;
			m_hue_info.hue_lut_b[i*4] = sin(radian)*128;
		}
		else
		{
			if (ket_point_y[i] == 0 && ket_point_x[i] > 0)
			{
				degree_temp = 0;
				m_hue_info.hue_lut_s[i*4] = ket_point_x[i];
			}
			else if (ket_point_y[i] == 0 && ket_point_x[i] < 0)
			{
				degree_temp = 180;
				m_hue_info.hue_lut_s[i*4] = ket_point_x[i]*(-1);
			}
			else if (ket_point_x[i] == 0 && ket_point_y[i] > 0)
			{
				degree_temp = 90;
				m_hue_info.hue_lut_s[i*4] = ket_point_y[i];
			}
			else if (ket_point_x[i] == 0 && ket_point_y[i] < 0)
			{
				degree_temp = 270;
				m_hue_info.hue_lut_s[i*4] = ket_point_y[i]*(-1);
			}
			
			//degree_temp = degree_temp - degree*i;
			degree_temp = degree*i-degree_temp;

			//记录每一个角度
			ket_point_degree[i] = degree_temp;

			radian = degree_temp * PI / 180;//弧度
			radian_ch = cos(radian)*128;
			if (radian_ch == 128)
			{
				radian_ch = 127;
			}
			m_hue_info.hue_lut_a[i*4] = radian_ch;
			radian_ch = sin(radian)*128;
			if (radian_ch == 128)
			{
				radian_ch = 127;
			}
			m_hue_info.hue_lut_b[i*4] = radian_ch;
		}
		
	}

	//最后一点和第一点重合的
	ket_point_degree[16] = ket_point_degree[0];
	m_hue_info.hue_lut_a[64] = m_hue_info.hue_lut_a[0];
	m_hue_info.hue_lut_b[64] = m_hue_info.hue_lut_b[0];
	m_hue_info.hue_lut_s[64] = m_hue_info.hue_lut_s[0];

	idex = 0;
	//获取中间的三个点
	for(i = 0; i < KEY_POINT_NUM-1; i++)
	{
		temp_s_2 = (m_hue_info.hue_lut_s[(i+1)*4 ] + m_hue_info.hue_lut_s[i*4])/2;
		temp_s_1 = (temp_s_2 + m_hue_info.hue_lut_s[i*4])/2;
		temp_s_3 = (m_hue_info.hue_lut_s[(i+1)*4] + temp_s_2)/2;

		idex++;

		degree_temp_ch = ket_point_degree[i + 1] - ket_point_degree[i];


		if (degree_temp_ch > 270)
			degree_temp_ch = degree_temp_ch - 360;
		else if (degree_temp_ch < -270)
			degree_temp_ch = degree_temp_ch + 360;
		
			
		degree_temp_ch = degree_temp_ch/4;
		degree_temp_last = ket_point_degree[i];


		degree_temp_last = degree_temp_last + degree_temp_ch;
		radian = degree_temp_last * PI / 180;//弧度
		radian_ch = cos(radian)*128;
		if (radian_ch == 128)
		{
			radian_ch = 127;
		}
		m_hue_info.hue_lut_a[idex] = radian_ch;
		radian_ch = sin(radian);
		radian_ch = radian_ch*128;
		if (radian_ch == 128)
		{
			radian_ch = 127;
		}
		m_hue_info.hue_lut_b[idex] = radian_ch;
		m_hue_info.hue_lut_s[idex] = temp_s_1;
		idex++;

		degree_temp_last = degree_temp_last + degree_temp_ch;
		radian = degree_temp_last * PI / 180;//弧度
		radian_ch = cos(radian)*128;
		if (radian_ch == 128)
		{
			radian_ch = 127;
		}
		m_hue_info.hue_lut_a[idex] = radian_ch;
		radian_ch = sin(radian)*128;
		if (radian_ch == 128)
		{
			radian_ch = 127;
		}
		m_hue_info.hue_lut_b[idex] = radian_ch;
		m_hue_info.hue_lut_s[idex] = temp_s_2;
		idex++;

		degree_temp_last = degree_temp_last + degree_temp_ch;
		radian = degree_temp_last * PI / 180;//弧度
		radian_ch = cos(radian)*128;
		if (radian_ch == 128)
		{
			radian_ch = 127;
		}
		m_hue_info.hue_lut_a[idex] = radian_ch;
		radian_ch = sin(radian)*128;
		if (radian_ch == 128)
		{
			radian_ch = 127;
		}
		m_hue_info.hue_lut_b[idex] = radian_ch;
		m_hue_info.hue_lut_s[idex] = temp_s_3;
		idex++;
	}
}

BOOL CHUE_lineDlg::check_all_data_is_zero() 
{	
	BOOL ret = FALSE;
	UINT i = 0;

	for (i = 0; i < 65; i++)
	{
		if (m_hue_info.hue_lut_a[i] != 0)
		{
			ret = TRUE;
			break;
		}
		if (m_hue_info.hue_lut_b[i] != 0)
		{
			ret = TRUE;
			break;
		}
		if (m_hue_info.hue_lut_s[i] != 0)
		{
			ret = TRUE;
			break;
		}
	}

	return ret;
}


BOOL CHUE_lineDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GdiplusStartupInput m_gdiplusStartupInput;
    GdiplusStartup(&m_pGdiToken,&m_gdiplusStartupInput,NULL);
	
	m_drag = FALSE;
	ZeroMemory(m_level, sizeof(m_level));
	m_moveflag = -1;
	
	
	CDC* pDC = GetDC();
	
	m_MemDC.CreateCompatibleDC(pDC);
	
	m_MemBitmap.CreateCompatibleBitmap(pDC, 610, 610);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	ReleaseDC(pDC);
	
	this->GetClientRect(&m_Rect);
	
	m_CurveRect.SetRect(m_Rect.left + 50, m_Rect.top + 20, 
		m_Rect.left + 50 + HUE_LINE_WINDOW_WIDTH, m_Rect.top + 20 + HUE_LINE_WINDOW_HEIGHT);
	
	m_CurveFrameRect.left = m_CurveRect.left - 2;
	m_CurveFrameRect.right = m_CurveRect.right + 2;
	m_CurveFrameRect.top = m_CurveRect.top - 2;
	m_CurveFrameRect.bottom = m_CurveRect.bottom + 2 ;
	
	
	m_handCursor = ::LoadCursor(NULL, IDC_HAND);
	m_show_backcolor = TRUE;

	//设置默认的关键点
	if (check_all_data_is_zero())
	{
		SetDataValue();
	}
	else
	{
		Set_init_DataValue(); 
	}

	SetKeyPts(&m_keyPts, ket_point_x, ket_point_y);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#define CURVE_LINE_COLOR		Color(0, 0, 0)		//BLACK
#define CONTROL_POINT_COLOR		Color(255, 0, 0)	//Red
#define BASE_LINE_COLOR			Color(150, 150, 150)

vector<CPoint>* CHUE_lineDlg::GetKeyPts(void)
{
	return &m_keyPts;
}

void CHUE_lineDlg::Get_Key_point(void)
{
	vector<CPoint> *pts_tmp = NULL;
	int i = 0;
	int size = 0;
	int index = 0;
	double temp = 0;

	int step = HUE_LINE_WINDOW_WIDTH / LINE_LEVEL_NUM;

	pts_tmp = GetKeyPts();
	size = (*pts_tmp).size();
	for (i=0; i<size; i++)
	{
		temp = ((*pts_tmp)[i].x - HUE_LINE_WINDOW_WIDTH/step)/step;
		ket_point_x[i]= temp;

		temp = (HUE_LINE_WINDOW_WIDTH/step - (*pts_tmp)[i].y)/step;
		ket_point_y[i] = temp;
	}
}



void CHUE_lineDlg::Refresh(void)
{
	InvalidateRect(m_CurveRect, FALSE);
}

void CHUE_lineDlg::SetKeyPts(vector<CPoint>* keypts, T_S16 *key_x, T_S16 *key_y)
{
	int cnt = 0, i = 0;

	(*keypts).clear();

	int step = HUE_LINE_WINDOW_WIDTH / LINE_LEVEL_NUM;

	for (i = 0; i < HUE_LINE_KEY_POINT; i++)
	{
		(*keypts).push_back(CPoint(HUE_LINE_WINDOW_WIDTH/step + step * key_x[i], HUE_LINE_WINDOW_WIDTH/step - step * key_y[i]));
	}
}

#define CLIP255_color(x) ((x)>0?((x)<255?(x):255):0)
void CHUE_lineDlg::Set_background_color(void) 
{
	T_S16 x = 0;
	T_S16 y = 0;
	T_S16 Y = 128, R = 0, G = 0, B = 0, i = 0, U = 0, V = 0;

	
	for (y =  0, V = HUE_LINE_X_MAX; y < HUE_LINE_WINDOW_WIDTH && V > HUE_LINE_X_MIN; V--)
	{
		for (x = 0, U = HUE_LINE_X_MIN; x < HUE_LINE_WINDOW_WIDTH && U < HUE_LINE_X_MAX; U++)
		{
			R = CLIP255_color((256*Y + 351*V + 128)>>8);
			G = CLIP255_color((256*Y - 86*U - 179*V + 128)>>8);
			B = CLIP255_color((256*Y + 443*U + 128)>>8);
			m_MemDC.SetPixel(x, y, RGB(R, G, B));
			m_MemDC.SetPixel(x, y+1, RGB(R, G, B));
			x++;
			m_MemDC.SetPixel(x, y, RGB(R, G, B));
			m_MemDC.SetPixel(x, y+1, RGB(R, G, B));
			x++;

		}
		y = y + 2;
	}

}

void CHUE_lineDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 __super::OnPaint()

	//draw frame
	dc.DrawEdge(m_CurveFrameRect, EDGE_BUMP, BF_RECT);
	
	if(m_show_backcolor)
	{
		Set_background_color();
		m_show_backcolor = FALSE;
	}
	
	//m_MemDC.FillSolidRect(0, 0, m_CurveRect.Width(), m_CurveRect.Height(), RGB(189, 252, 201));
	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);
	
	CBrush brush(RGB(255, 0, 0));
	CBrush* pOldBrush;
	
	pOldBrush = m_MemDC.SelectObject(&brush);
	
	CPen pen(PS_SOLID, 4, RGB(255, 0, 0));
	CPen* pOldPen = m_MemDC.SelectObject(&pen);
	Graphics graphics(m_MemDC);
	
	//draw base line
	Pen gdiBaseLinePen(BASE_LINE_COLOR);
	graphics.DrawLine(&gdiBaseLinePen, Point(0, HUE_LINE_WINDOW_WIDTH/2), Point(HUE_LINE_WINDOW_WIDTH-1, HUE_LINE_WINDOW_WIDTH/2));
	graphics.DrawLine(&gdiBaseLinePen, Point(HUE_LINE_WINDOW_WIDTH/2, 0), Point(HUE_LINE_WINDOW_WIDTH/2, HUE_LINE_WINDOW_WIDTH));
	
	int steplen = HUE_LINE_WINDOW_WIDTH / 16;
	
	for (int i=1; i<16; i++)
	{
		graphics.DrawLine(&gdiBaseLinePen, Point(HUE_LINE_WINDOW_WIDTH/2-1, steplen*i), Point(HUE_LINE_WINDOW_WIDTH/2+1, steplen*i));
		graphics.DrawLine(&gdiBaseLinePen, Point(steplen*i, HUE_LINE_WINDOW_WIDTH/2-1), Point(steplen*i, HUE_LINE_WINDOW_WIDTH/2+1));
	}

	SolidBrush gdiControlPosBrush1(BASE_LINE_COLOR1);
	SolidBrush gdiControlPosBrush2(BASE_LINE_COLOR2);
	SolidBrush gdiControlPosBrush3(BASE_LINE_COLOR3);
	SolidBrush gdiControlPosBrush4(BASE_LINE_COLOR4);
	SolidBrush gdiControlPosBrush5(BASE_LINE_COLOR5);
	SolidBrush gdiControlPosBrush6(BASE_LINE_COLOR6);
	SolidBrush gdiControlPosBrush7(BASE_LINE_COLOR7);
	SolidBrush gdiControlPosBrush8(BASE_LINE_COLOR8);
	SolidBrush gdiControlPosBrush9(BASE_LINE_COLOR9);
	SolidBrush gdiControlPosBrush10(BASE_LINE_COLOR10);
	SolidBrush gdiControlPosBrush11(BASE_LINE_COLOR11);
	SolidBrush gdiControlPosBrush12(BASE_LINE_COLOR12);
	SolidBrush gdiControlPosBrush13(BASE_LINE_COLOR13);
	SolidBrush gdiControlPosBrush14(BASE_LINE_COLOR14);
	SolidBrush gdiControlPosBrush15(BASE_LINE_COLOR15);
	SolidBrush gdiControlPosBrush16(BASE_LINE_COLOR16);

	
	vector<CPoint>::iterator iter2 = m_keyPts.begin();
	
	for (i = 0; i < 16 && iter2 != m_keyPts.end(); i++, ++iter2) 
	{
		Point points[4] = {Point(iter2->x, iter2->y - 3), Point(iter2->x - 3, iter2->y), 
			Point(iter2->x, iter2->y + 3), Point(iter2->x + 3, iter2->y)};
		switch (i)
		{
		case 0:
			graphics.FillPolygon(&gdiControlPosBrush1, points, 4);
			break;
		case 1:
			graphics.FillPolygon(&gdiControlPosBrush2, points, 4);
			break;
		case 2:
			graphics.FillPolygon(&gdiControlPosBrush3, points, 4);
			break;
		case 3:
			graphics.FillPolygon(&gdiControlPosBrush4, points, 4);
			break;
		case 4:
			graphics.FillPolygon(&gdiControlPosBrush5, points, 4);
			break;
		case 5:
			graphics.FillPolygon(&gdiControlPosBrush6, points, 4);
			break;
		case 6:
			graphics.FillPolygon(&gdiControlPosBrush7, points, 4);
			break;
		case 7:
			graphics.FillPolygon(&gdiControlPosBrush8, points, 4);
			break;
		case 8:
			graphics.FillPolygon(&gdiControlPosBrush9, points, 4);
			break;
		case 9:
			graphics.FillPolygon(&gdiControlPosBrush10, points, 4);
			break;
		case 10:
			graphics.FillPolygon(&gdiControlPosBrush11, points, 4);
			break;
		case 11:
			graphics.FillPolygon(&gdiControlPosBrush12, points, 4);
			break;
		case 12:
			graphics.FillPolygon(&gdiControlPosBrush13, points, 4);
			break;
		case 13:
			graphics.FillPolygon(&gdiControlPosBrush14, points, 4);
			break;
		case 14:
			graphics.FillPolygon(&gdiControlPosBrush15, points, 4);
			break;
		case 15:
			graphics.FillPolygon(&gdiControlPosBrush16, points, 4);
			break;
		default:
			graphics.FillPolygon(&gdiControlPosBrush1, points, 4);
			break;
		}
	}

	m_MemDC.SelectObject(pOldBrush);
	brush.DeleteObject();
	m_MemDC.SelectObject(pOldPen);
	pen.DeleteObject();
	m_MemDC.SetBkMode(oldBkMode);
	
	dc.BitBlt(m_CurveRect.left, m_CurveRect.top, m_CurveRect.Width(), m_CurveRect.Height(), &m_MemDC, 0, 0, SRCCOPY);
	
	//连线
	iter2 = m_keyPts.begin();
	CPen pen_black(PS_SOLID, 1, RGB(0, 0, 0));
	dc.SelectObject(&pen_black);
	dc.MoveTo(iter2->x + m_CurveRect.left, iter2->y + m_CurveRect.top);
	for (i = 0; i < 16 && iter2 != m_keyPts.end(); i++, ++iter2) 
	{		
		dc.LineTo(iter2->x+m_CurveRect.left, iter2->y+m_CurveRect.top);
		if (i == 15)
		{
			break;
		}	
	}
	iter2 = m_keyPts.begin();
	dc.LineTo(iter2->x+m_CurveRect.left, iter2->y+m_CurveRect.top);

	
}

void CHUE_lineDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	Get_Key_point();

	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}
	
	//卸载gdi+
    GdiplusShutdown(m_pGdiToken);
	
	CDialog::OnClose();
	DestroyWindow();
}

void CHUE_lineDlg::Close() 
{
	OnClose();
}

T_U8 CHUE_lineDlg::account_s_value( T_S16 ket_x, T_S16 ket_y) 
{
	long double degree_temp = 0, radian = 0, temp_a = 0, degree = 0;
	T_U8 s_value = 0;

	if (ket_y != 0 && ket_x != 0)
	{
		if (ket_y > 0 && ket_x > 0)
		{
			temp_a = (long double)ket_y/(long double)ket_x;
			degree_temp = atan(temp_a)*180/PI;   //角度
			
		}
		else if(ket_y > 0 && ket_x < 0)
		{
			temp_a = (long double)ket_x/(long double)ket_y;
			degree_temp = (-1)*atan(temp_a)*180/PI;   //角度
			degree_temp = degree_temp + 90;
		}
		else if(ket_y < 0 && ket_x < 0)
		{
			temp_a = (long double)ket_y/(long double)ket_x;
			degree_temp = atan(temp_a)*180/PI;   //角度
			degree_temp = degree_temp + 180;
		}
		else
		{
			temp_a = (long double)ket_x/(long double)ket_y;
			degree_temp = (-1)*atan(temp_a)*180/PI;   //角度
			degree_temp = degree_temp + 270;	
		}
		
		radian = degree_temp * PI / 180;//弧度
		s_value = ket_x/cos(radian);
	}
	else
	{
		if (ket_y == 0 && ket_x > 0)
		{
			degree_temp = 0;
			s_value = ket_x;
		}
		else if (ket_y == 0 && ket_x < 0)
		{
			degree_temp = 180;
			s_value = ket_x*(-1);
		}
		else if (ket_x == 0 && ket_y > 0)
		{
			degree_temp = 90;
			s_value= ket_y;
		}
		else if (ket_x == 0 && ket_y < 0)
		{
			degree_temp = 270;
			s_value = ket_y*(-1);
		}
	}

	return s_value;
}

void CHUE_lineDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_CurveRect.PtInRect(point)) {
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	//Invalidate();
	
	CPoint InPoint;
	InPoint.x = point.x - m_CurveRect.left;
	InPoint.y = point.y - m_CurveRect.top;
	
	for (unsigned int i = 1; i < m_keyPts.size(); ++i)
	{
		if (InPoint.x > m_keyPts[i - 1].x + CONTROL_POINT_APART && InPoint.y > 0 && 
			InPoint.x < m_keyPts[i].x - CONTROL_POINT_APART && InPoint.y < m_CurveRect.Height())
		{
			//vector<CPoint>::iterator iterInsert = m_keyPts.begin() + i;
			//m_keyPts.insert(iterInsert, InPoint);
			//m_drag = TRUE;
			//m_moveflag = i;
			//::SetCursor(m_handCursor);
			//InvalidateRect(m_CurveRect, FALSE);
		}
	}
	
	vector<CPoint>::iterator iter = m_keyPts.begin();
	CRect rect(InPoint.x - 3, InPoint.y - 3, InPoint.x + 3, InPoint.y + 3);
	for (i = 0; iter != m_keyPts.end(); ++iter, ++i) {
		if (rect.PtInRect(*iter)) {
			m_drag = TRUE;
			m_moveflag = i;
			m_keyPts[m_moveflag] = InPoint;
		}
	}
	T_U8 s_value;
	T_S16 ket_x,  ket_y;
	CString strCoord;
	CWnd * pCoordTextX = GetDlgItem(IDC_STATIC_X);
	CWnd * pCoordTextY = GetDlgItem(IDC_STATIC_Y);
	CWnd * pCoordTextS = GetDlgItem(IDC_STATIC_S);
	s_value = m_CurveRect.Width();
	s_value = m_CurveRect.Height();

	//ket_x = InPoint.x * m_level_num / m_CurveRect.Width() + HUE_LINE_X_MIN;
	ket_x = InPoint.x/2 + HUE_LINE_X_MIN;

	if(InPoint.y >= 0 && InPoint.y < 256)
	{
		ket_y = HUE_LINE_X_MAX - InPoint.y/2;
	}
	else
	{
		ket_y = (HUE_LINE_WINDOW_HEIGHT - InPoint.y)/2 + HUE_LINE_Y_MIN;
	}

	if (ket_y > HUE_LINE_X_MAX)
	{
		ket_y = HUE_LINE_X_MAX;
	}

	if (ket_y < HUE_LINE_Y_MIN)
	{
		ket_y = HUE_LINE_Y_MIN;
	}

	//ket_y = HUE_LINE_Y_MAX - InPoint.y * (HUE_LINE_Y_MAX - HUE_LINE_Y_MIN) / m_CurveRect.Height();
	s_value = account_s_value(ket_x, ket_y);

	strCoord.Format("%d", ket_x);
	pCoordTextX->SetWindowText(strCoord);
	strCoord.Format("%d", ket_y);
	pCoordTextY->SetWindowText(strCoord);
	strCoord.Format("%d", s_value);
	pCoordTextS->SetWindowText(strCoord);

	CDialog::OnLButtonDown(nFlags, point);
}

void CHUE_lineDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (m_drag)
		m_drag = FALSE;

	//m_show_backcolor = TRUE;

	CDialog::OnLButtonUp(nFlags, point);
}

void CHUE_lineDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_CurveRect.PtInRect(point)) {
		CDialog::OnMouseMove(nFlags, point);
		m_drag = FALSE;
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_CurveRect.left;
	InPoint.y = point.y - m_CurveRect.top;
	
	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < m_keyPts.size() - 1) {
		//if (InPoint.x > m_keyPts[m_moveflag - 1].x + CONTROL_POINT_APART && 
		//	InPoint.x < m_keyPts[m_moveflag + 1].x - CONTROL_POINT_APART)
		{
			m_keyPts[m_moveflag] = InPoint;
		}
		//else {
			//vector<CPoint>::iterator iterRemove = m_keyPts.begin() + m_moveflag;
			//m_keyPts.erase(iterRemove);
			//m_moveflag = -1;
			//m_drag = FALSE;
	//	}
	}
	
	if (m_drag && m_moveflag == 0 && InPoint.x < m_keyPts[1].x - CONTROL_POINT_APART)
		m_keyPts[0] = InPoint;
	
    if (m_drag && m_moveflag == m_keyPts.size() - 1 && 
		InPoint.x > m_keyPts[m_keyPts.size() - 2].x + CONTROL_POINT_APART)
        m_keyPts[m_moveflag] = InPoint;
	
	InvalidateRect(m_CurveRect, FALSE);
	
	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < m_keyPts.size()) 
	{
		T_U8 s_value;
		T_S16 ket_x,  ket_y;
		CString strCoord;
		CWnd * pCoordTextX = GetDlgItem(IDC_STATIC_X);
		CWnd * pCoordTextY = GetDlgItem(IDC_STATIC_Y);
		CWnd * pCoordTextS = GetDlgItem(IDC_STATIC_S);
		
		ket_x = InPoint.x/2 + HUE_LINE_X_MIN;
		
		if(InPoint.y >= 0 && InPoint.y < 256)
		{
			ket_y = HUE_LINE_X_MAX - InPoint.y/2;
		}
		else
		{
			ket_y = (HUE_LINE_WINDOW_HEIGHT - InPoint.y)/2 + HUE_LINE_Y_MIN;
		}

		if (ket_y > HUE_LINE_X_MAX)
		{
			ket_y = HUE_LINE_X_MAX;
		}
		
		if (ket_y < HUE_LINE_Y_MIN)
		{
			ket_y = HUE_LINE_Y_MIN;
		}

		//ket_x = InPoint.x * m_level_num / m_CurveRect.Width() + HUE_LINE_X_MIN;
		//ket_y = HUE_LINE_Y_MAX - InPoint.y * (HUE_LINE_Y_MAX - HUE_LINE_Y_MIN) / m_CurveRect.Height();
		s_value = account_s_value(ket_x, ket_y);
		
		strCoord.Format("%d", ket_x);
		pCoordTextX->SetWindowText(strCoord);
		strCoord.Format("%d", ket_y);
		pCoordTextY->SetWindowText(strCoord);
		strCoord.Format("%d", s_value);
		pCoordTextS->SetWindowText(strCoord);
	}

	m_show_backcolor = TRUE;

	CDialog::OnMouseMove(nFlags, point);
}

void CHUE_lineDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
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

void CHUE_lineDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CHUE_lineDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	if (m_drag) {
		::SetCursor(m_handCursor);
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CHUE_lineDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) {
		SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CHUE_lineDlg::OnButtonReset() 
{
	// TODO: Add your control notification handler code here
	ZeroMemory(m_level, sizeof(m_level));
	
	m_drag = FALSE;
	m_moveflag = -1;
	
	m_keyPts.clear();
	
	m_show_backcolor = TRUE;
	
	//设置默认的关键点
#if 0
	if (m_hue_info.hue_sat_en)
	{
		SetDataValue();
	}
	else
#endif
	{
		Set_init_DataValue(); 
	}

	SetKeyPts(&m_keyPts, ket_point_x, ket_point_y);
	
	InvalidateRect(m_CurveRect, FALSE);
}

void CHUE_lineDlg::OnButtonRead() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(TRUE, "*.txt", NULL, OFN_HIDEREADONLY,
		"Config File(*.txt)|*.txt|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}
	
	if (file.GetLength() != sizeof(AK_ISP_HUE))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}
	
	file.Read(&m_hue_info, sizeof(AK_ISP_HUE));
	file.Close();

	m_keyPts.clear();

	
	m_drag = FALSE;
	m_moveflag = -1;	
	m_show_backcolor = TRUE;
	
	SetDataValue();
	
	SetKeyPts(&m_keyPts, ket_point_x, ket_point_y);

	InvalidateRect(m_CurveRect, FALSE);
}

void CHUE_lineDlg::OnButtonWrite() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(FALSE, "*.txt", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"Config File(*.txt)|*.txt|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeCreate|CFile::modeWrite, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	Get_Key_point();
	
	GetDataValue();
	
	file.Write(&m_hue_info, sizeof(AK_ISP_HUE));
	
	file.Close();
}
