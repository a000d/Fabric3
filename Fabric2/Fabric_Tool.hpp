#pragma once
#include "utils.hpp"
#include <filesystem>
#include <unordered_map>
#include "String_Utils.hpp"
#include "Polyline_Sweep_Cylinder.hpp"
#include <iostream>
#include <fstream>
#include <thread>

using namespace std;
namespace fs = std::filesystem;
namespace su = String_Utils;


FZ_Data read_fz(string fz_path) {
    size_t size = fs::directory_entry(fz_path).file_size();
    char* buffer = new char[size];

    ifstream fin(fz_path,ios::binary|ios::in);
    fin.read(buffer, size);
    fin.close();

    int version = _u8(buffer,0);
    int card_count = _u8(buffer,1);

    int Z = _u16(buffer,2);
    int S = _u16(buffer,4);

    vector<vector<int>> jb_value =	{ {_u8(buffer,6 + 0) ,_u8(buffer,6 + 1)} ,{_u8(buffer,6 + 2) ,_u8(buffer,6 + 3)},
                                    {_u8(buffer,6 + 4) ,_u8(buffer,6 + 5)} ,{_u8(buffer,6 + 6) ,_u8(buffer,6 + 7)} };

    int bytes_per_step = (Z + 7) / 8;

    vector<vector<int>> actions;
    int base = 14;

    for (int i = 0; i < S;i++) {
        int start = base + i * bytes_per_step;
        int end = start + bytes_per_step;

        vector<int> unpacked = bytes_unpack(buffer,start,end);

        ASSERT(unpacked.size() == Z);


        actions.push_back(unpacked);

    }
    delete[](buffer);

    return FZ_Data(version, Z, S, jb_value, actions, bytes_per_step);
}

void Draw_Line(const Unit_Table& unit_table,const float stretch,vector<vector<v3_f>>& curve_list) {

    int width = unit_table.width;
    int height = unit_table.height;


    //
    curve_list.clear();

    curve_list.reserve(width);

    for (int x = 0; x < width; x++) {
        curve_list.push_back(vector<v3_f>{});
    }


    auto draw_line_item = [&curve_list,&unit_table,&height,&width,stretch](int x)->void {

        //std::cout << su::fmt("\r绘制线圈 {}/{}       ", { x ,width });

        vector<v3_f> curve_point_list;
        curve_point_list.reserve(height);

        for (int y = 0; y < height; y++) {

            const Act_Unit& unit = unit_table.table[x][y];
            const int front = unit.front;
            const int back = unit.back;

            if (front == back) {
                //std::cout << "direction err:"<< x<<" "<<y<<" " << front <<" " << back << endl;
                continue;
            }

            v3_f center = { x - f(front + back) / 2, f(y) / f(1.5), 0.0 };

            DIRECTION in_dir;
            DIRECTION out_dir;

            if (y == 0) {
                in_dir = mid_v;
            }
            else if (unit_table.table[x][y - 1].Get_Center().x < center.x) {
                in_dir = min_v;
            }
            else if (unit_table.table[x][y - 1].Get_Center().x > center.x) {
                in_dir = add_v;
            }
            else {
                in_dir = mid_v;
            }

            if (y == height - 1) {
                out_dir = mid_v;
            }
            else if (unit_table.table[x][y + 1].Get_Center().x < center.x) {
                out_dir = min_v;
            }
            else if (unit_table.table[x][y + 1].Get_Center().x > center.x) {
                out_dir = add_v;
            }
            else {
                out_dir = mid_v;
            }

            vector<v3_f> unit_points;
            if (front > back) {
                unit_points = Get_Curve(center, n_in, in_dir, out_dir, stretch);
            }
            else if (back > front) {
                unit_points = Get_Curve(center, p_in, in_dir, out_dir, stretch);
            }
            else {
                std::cout << "direction err" << endl;
                continue;
            }

            //for (int i = 0; i < unit_points.size(); i++) {
            //	v3_f p = unit_points[i];

            //	while (true) {
            //		string k = p.Get_Hash_str();
            //		if (point_location_map.count(k) == 0) {
            //			point_location_map[k] = 1;
            //			break;
            //		}
            //		else {
            //			p = { p.x + f(0.03),p.y,p.z + f(0.02) };

            //			unit_points[i] = p;

            //		}
            //	}


            //}


            curve_point_list.insert(curve_point_list.end(), unit_points.begin(), unit_points.end());


        }

        curve_list[x] = curve_point_list;

    };

    // for (int x = 0; x < width;x++) {
    //     // std::thread(draw_line_item,x));
    //     draw_line_item(x);
    // }

    vector<std::thread> thread_list;
    for (int x = 0; x < width;x++) {
        thread_list.push_back(std::thread(draw_line_item,x));
    }
    for (int x = 0; x < width; x++) {
        thread_list[x].join();
    }

    // 处理重叠点

    unordered_map<double, int> point_location_map;


    for (auto& curve : curve_list) {
        for (v3<float>& p : curve) {


            while (true) {

                double k = p.Get_Hash_str();

                if (point_location_map.count(k) == 0) {
                    point_location_map[k] = 1;
                    break;
                }
                else {
                    p = { p.x + f(0.03),p.y+ f(0.01),p.z + f(0.03) };

                }
            }
        }
    }

    point_location_map.clear();

}

void Sweep(int i, float sweep_width, int segments, const vector<v3_f>& curve,
           vector<v3_f>& vertices_result,
           vector<v3_f>& normal_result,
           vector<v3<unsigned int>>& faces_result) {

    //std::cout << su::fmt("\r扫描线圈 {}/{}       ", { i ,curve_list.size() });

    //const vector<v3_f>& curve = curve_list[i];

    //vector<v3_f>& vertices = vertices_result_list[i];
    //vector<v3<unsigned int>>& faces = faces_result_list[i];

    Polyline_Sweep_Cylinder(curve, sweep_width, segments, vertices_result, faces_result);

    normal_result.clear();
    normal_result.reserve(vertices_result.size());
    for (int i = 0; i < vertices_result.size();i++) {
        normal_result.push_back({0,0,0});
    }

    for (int i = 0; i < faces_result.size();i++) {

        uint idx0 = faces_result[i].x;
        uint idx1 = faces_result[i].y;
        uint idx2 = faces_result[i].z;

        v3_f v0 = vertices_result[idx0];
        v3_f v1 = vertices_result[idx1];
        v3_f v2 = vertices_result[idx2];

        v3_f edge1 = v1 - v0;
        v3_f edge2 = v2 - v0;

        v3_f face_normal;

        try {
            face_normal = cross(edge1, edge2);
        }
        catch (...) {
            cout << "sth err" << endl;
        }

        normal_result[idx0] = normal_result[idx0] + face_normal;
        normal_result[idx1] = normal_result[idx1] + face_normal;
        normal_result[idx2] = normal_result[idx2] + face_normal;


    }
    for (int i = 0; i < normal_result.size(); i++) {
        normal_result[i] = normal_result[i].Get_Norm();

        if(std::isnan(normal_result[i].y) ||std::isinf(normal_result[i].y) ){
            cout<<1<<endl;
        }

    }

}

class Fabric_Tool{

public:
    Fabric_Tool(){


    }

    void Generate_Curve(string fz_path,float sweep_width,int segments,string jb_value_override,float stretch,
                        vector<vector<v3_f>>& vertices_result_list,
                        vector<vector<v3<uint>>>& faces_result_list,
                        vector<vector<v3_f>>& normal_result_list){

        try {
            if (segments<3) {
                throw 0;
            }
            if (!fs::directory_entry(fz_path).exists()) {
                throw 0;
            }
            std::cout << "fz_path\t\t: "			<< fz_path << endl;
            std::cout << "sweep_width\t\t: "		<< sweep_width << endl;
            std::cout << "segments	\t\t: "			<< segments << endl;
            std::cout << "jb_value_override	\t\t: " << jb_value_override << endl;
            std::cout << "stretch	\t\t: "			<< stretch << endl;
        }
        catch (...) {
            std::cout<<"input wrong!" << endl;
            return;
        }

        FZ_Data fz_data = read_fz(fz_path);

        int fz_width = fz_data.Z;
        int fz_height = fz_data.S / 2;
        vector<vector<int>> fz_jb_value = fz_data.jb_value;

        if (jb_value_override!="-1") {
            try {
                fz_jb_value[0][0] = atoi(jb_value_override.substr(0,1).c_str());
                fz_jb_value[0][1] = atoi(jb_value_override.substr(1,1).c_str());
                fz_jb_value[1][0] = atoi(jb_value_override.substr(2,1).c_str());
                fz_jb_value[1][1] = atoi(jb_value_override.substr(3,1).c_str());
                fz_jb_value[2][0] = atoi(jb_value_override.substr(4,1).c_str());
                fz_jb_value[2][1] = atoi(jb_value_override.substr(5,1).c_str());
                fz_jb_value[3][0] = atoi(jb_value_override.substr(6,1).c_str());
                fz_jb_value[3][1] = atoi(jb_value_override.substr(7,1).c_str());


            }
            catch (...) {
                std::cout << "input wrong!" << endl;
                return;
            }

        }
        std::cout << fz_jb_value[0][0] << " ";
        std::cout << fz_jb_value[0][1] << " ";
        std::cout << fz_jb_value[1][0] << " ";
        std::cout << fz_jb_value[1][1] << " ";
        std::cout << fz_jb_value[2][0] << " ";
        std::cout << fz_jb_value[2][1] << " ";
        std::cout << fz_jb_value[3][0] << " ";
        std::cout << fz_jb_value[3][1] << " ";
        std::cout << endl;

        Unit_Table unit_table = { fz_width, fz_height, fz_jb_value };

        unit_table.Apply_Actions(fz_data.actions);
        unit_table.y_half_split();

        vector<vector<v3_f>> curve_list;
        Draw_Line(unit_table, stretch,curve_list);


        // vector<vector<v3_f>> vertices_result_list;
        // vector<vector<v3<uint>>> faces_result_list;
        // vector<vector<v3_f>> normal_result_list;

        vertices_result_list.clear();
        faces_result_list.clear();
        normal_result_list.clear();

        try{
            for (int i = 0; i < curve_list.size(); i++)
            {


                vector<v3_f> vertices_result;
                vector<v3<uint>> faces_result;
                vector<v3_f> normal_result;

                Sweep(i,sweep_width, segments, curve_list[i], vertices_result, normal_result, faces_result);
                vertices_result_list.push_back(vertices_result);
                faces_result_list.push_back(faces_result);
                normal_result_list.push_back(normal_result);

            }
        }
        catch (...) {
            cout << "err" << endl;
            return;
        }

        cout<<"DONE"<<endl;
    }


private:

public:









};






