#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<math.h>
#include "String_Utils.hpp"
namespace su = String_Utils;
using namespace std;

template<typename T>
struct v3 {
    T x;
    T y;
    T z;


    v3(T x = 0, T y = 0, T z = 0) {
        this->x = x;
        this->y = y;
        this->z = z;

    };
    v3 operator+(const v3& b) const {

        return { this->x + b.x,this->y + b.y,this->z + b.z };

    }
    v3 operator-(const v3& b) const {

        return { this->x - b.x,this->y - b.y,this->z - b.z };

    }
    v3 operator/(const float v) const {

        return { x / v,y / v,z / v };
    }
    v3 operator*(const float v) const {

        return { x * v,y * v,z * v };
    }


    float length() const {
        return sqrt(x * x + y * y + z * z);
    }

    v3 Get_Norm() {

        float l = length();
        return {x/l,y/l,z/l};
    }

    double Get_Hash_str() const{

        return (double)(x)*100000.0*100000.0 + (double)(y)*100000.0 + (double)(z);
    }

};

#define PI 3.1415926
#define v3_f v3<float>
#define ASSERT(v) if(!v){throw 0;}
#define f(v) (float)(v)
#define DLL_EXP extern"C" _declspec(dllexport)
typedef unsigned int uint;

enum DIRECTION {
    None=-1,
    p_in=0,
    n_in,
    add_v,
    mid_v,
    min_v,

};

v3_f cross(const v3_f& a, const v3_f& b) {

    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - b.x * a.y
    };
}

int  _u8(char* data,int offset) {
    return data[offset];
}
int  _u16(char* data, int offset) {

    char buf[2];
    buf[1] = data[offset];
    buf[0] = data[offset+1];

    short* t = (short*)buf;

    return *t;
}


class FZ_Data {

public:
    int version;
    int Z;
    int S;
    vector<vector<int>> jb_value;
    vector<vector<int>> actions;
    int bytes_per_step;

    FZ_Data(int version,
            int Z,
            int S,
            vector<vector<int>> jb_value,
            vector<vector<int>> actions,
            int bytes_per_step) {

        this->version = version;
        this->Z = Z;
        this->S = S;
        this->jb_value = jb_value;
        this->actions = actions;
        this->bytes_per_step = bytes_per_step;


    }
    FZ_Data(){}


};

vector<int> bytes_unpack(const char* buf, int start, int end) {

    int len = end - start;

    vector<int> res;
    res.reserve(len * 8);

    for (int pos = start; pos < end;pos++) {

        char b = buf[pos];

        for (int i = 0; i < 8;i++) {
            if (b & (0b00000001 << i)) {
                res.push_back(1);
            }
            else {
                res.push_back(0);
            }
        }


    }



    return res;

}


struct Act_Unit {
public:
    int x;
    int y;
    int front;
    int back;
public:
    Act_Unit(int x,int y,int front, int back):x(x),y(y),front(front),back(back) {


    }
    v3<float> Get_Center() const{
        return { (float)(x - (front + back) / 2), (float)(y / 1.5), 0.0};

    }


};


class Unit_Table {
public:
    int width;
    int height;
    vector<vector<int>> jb_value;
    vector<vector<Act_Unit>> table;

public:
    Unit_Table(int width,int height,const vector<vector<int>>& jb_value) {

        this->width = width;
        this->height = height;
        this->jb_value = jb_value;

        table.reserve(width);

        for (int x = 0; x < width; x++) {

            vector<Act_Unit> tmp_list;
            tmp_list.reserve(height);
            int jb_id = 0;

            for (int y = 0; y < height;y++) {

                Act_Unit unit = { x,y,jb_value[jb_id][0],jb_value[jb_id][1] };
                tmp_list.push_back(unit);
                jb_id++;
                if (jb_id>=4) {
                    jb_id = 0;
                }
            }
            table.push_back(tmp_list);

        }




    }

    void Apply_Actions(const vector<vector<int>>& actions) {

        for (int x = 0; x < width;x++) {
            for (int y = 0; y < height;y++) {

                table[x][y].front += actions[y * 2][x];
                table[x][y].back += actions[y * 2 + 1][x];
            }

        }
    }
    void y_half_split() {
        for (int x = 0; x < width; x++) {
            const vector<Act_Unit>& tmp = table[x];

            vector<Act_Unit> new_list;
            new_list.reserve(height/2);

            for (int y = 0; y < height / 2;y++) {
                new_list.push_back(tmp[y*2]);
            }
            table[x] = new_list;
        }

        height = height / 2;
    }




};

void c4d_MS(v3_f& new_point_0A, v3_f& new_point_0B,const v3_f& p0, const v3_f& pa,const v3_f& pb, float len=0.04) {

    if ((pa - p0).length()<len or (pb-p0).length()<len) {

        new_point_0A = {-999,-999,-999};
        new_point_0B = {-999,-999,-999};
        return;
    }

    v3_f dir_A = (pa - p0).Get_Norm();
    v3_f dir_B = (pb - p0).Get_Norm();

    new_point_0A = p0 + dir_A * len;
    new_point_0B = p0 + dir_B * len;


}

vector<v3_f> Get_Curve(const v3_f center, DIRECTION direction, DIRECTION in_dir, DIRECTION out_dir, float stretch) {

    vector<v3_f> curve_unit_template = {
                                        {f(99999 ) ,   f(99999)   ,   f(99999	)},
                                        {f(-0.2  ) ,   f(-0.58 )   ,   f(0		)},         // 两点重合，用于倒角
                                        {f(-0.2  ) ,   f(-0.58 )   ,   f(0		)},         // 两点重合，用于倒角
                                        {f(-0.4  ) ,   f(0.5  )   ,   f(-0.0333	)},
                                        {f(-0.376) ,   f(0.671)   ,   f(-0.0376	)},
                                        {f(-0.306) ,   f(0.821)   ,   f(-0.0528	)},
                                        {f(-0.2  ) ,   f(0.933)   ,   f(-0.064	)},
                                        {f(-0.069) ,   f(0.992)   ,   f(-0.0684	)},

                                        //{f(0) ,   f(0.992)   ,   f(-0.0684)},

                                        {f(0.069 ) ,   f(0.992)   ,   f(-0.0684	)},
                                        {f(0.2   ) ,   f(0.933)   ,   f(-0.064	)},
                                        {f(0.306 ) ,   f(0.821)   ,   f(-0.0528	)},
                                        {f(0.376 ) ,   f(0.671)   ,   f(-0.0376	)},
                                        {f(0.4   ) ,   f(0.5  )   ,   f(-0.0333	)},
                                        {f(0.2   ) ,   f(-0.58 )   ,   f(0		)},         // 两点重合，用于倒角
                                        {f(0.2   ) ,   f(-0.58 )   ,   f(0		)},         // 两点重合，用于倒角
                                        {f(99999 ) ,   f(99999)   ,   f(99999	)},
                                        };

    if (direction == p_in) {
        std::reverse(curve_unit_template.begin(), curve_unit_template.end());
    }


    if (in_dir == add_v) {
        curve_unit_template[0] = curve_unit_template[1] + v3_f(0.2, -0.2, -0.2);
    }
    else if (in_dir == min_v) {
        curve_unit_template[0] = curve_unit_template[1] + v3_f(-0.2, -0.2, -0.2);
    }
    else {
        curve_unit_template[0] = curve_unit_template[1] + v3_f(0, -0.2, -0.2);
    }

    if (out_dir == add_v) {
        curve_unit_template[curve_unit_template.size() - 1] = curve_unit_template[curve_unit_template.size() - 2] + v3_f(0.2, 0.2, -0.2);
    }
    else if (out_dir == min_v) {
        curve_unit_template[curve_unit_template.size() - 1] = curve_unit_template[curve_unit_template.size() - 2] + v3_f(-0.2, 0.2, -0.2);
    }
    else {
        curve_unit_template[curve_unit_template.size() - 1] = curve_unit_template[curve_unit_template.size() - 2] + v3_f(0, 0.2, -0.2);
    }


    v3_f a;
    v3_f b;

    c4d_MS(a,b, curve_unit_template[1], curve_unit_template[0], curve_unit_template[3], 0.08);
    if (a.x==-999) {
        curve_unit_template.erase(curve_unit_template.begin()+1);
    }
    else {
        curve_unit_template[1] = a;
        curve_unit_template[2] = b;
    }


    c4d_MS(a, b, curve_unit_template[curve_unit_template.size()-2], curve_unit_template[curve_unit_template.size() - 4], curve_unit_template[curve_unit_template.size() - 1], f(0.08));
    if (a.x==-999) {
        curve_unit_template.erase(curve_unit_template.begin() + curve_unit_template.size() -2);
    }
    else {
        curve_unit_template[curve_unit_template.size() -3] = a;
        curve_unit_template[curve_unit_template.size() -2] = b;
    }



    //////////////////////拉伸 x轴缩放////////////////////
    for (v3_f& p : curve_unit_template) {
        p.x = p.x / stretch;

    }


    //////////////////////////////////////////////////////////////
    vector<v3_f> result;
    result.reserve(curve_unit_template.size());

    float cx = center.x;
    float cy = center.y;
    float cz = center.z;

    for (const v3_f& p: curve_unit_template) {

        result.push_back({ p.x + cx, p.y + cy, p.z + cz });
    }

    return result;
}

void OBJ_Concat(

    const vector<vector<v3_f>>&     vertices_list, const vector<vector<v3<unsigned int>>>&  faces_list,
    vector<v3_f>& vertices, vector<v3<unsigned int>>& faces) {

    vertices.clear();
    faces.clear();

    for (int i = 0; i < vertices_list.size();i++) {
        vertices.insert(vertices.end(), vertices_list[i].begin(), vertices_list[i].end());
    }

    int index_offset = 0;
    for (int i = 0; i < faces_list.size(); i++) {

        for (const v3<unsigned int>& f: faces_list[i]) {

            faces.push_back({f.x+ index_offset,f.y+ index_offset,f.z+ index_offset });

        }
        index_offset += faces_list[i].size();

    }

}



void Faces_to_Obj(vector<v3_f> vertices, vector<v3<unsigned int>> faces,string name) {

    string obj_content;

    for (const v3_f& p : vertices) {

        obj_content += su::fmt("v {} {} {}\n", { p.x,p.y,p.z });

    }

    for (const v3<unsigned int>& p : faces) {

        obj_content += su::fmt("f {} {} {}\n", { p.x,p.y,p.z });

    }
    ofstream fout(name, ios::out | ios::binary);
    fout.write(obj_content.c_str(), obj_content.size());
    fout.close();


}
