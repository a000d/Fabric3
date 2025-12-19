#pragma once
#include "utils.hpp"
#include <filesystem>
#include <unordered_map>
#include "String_Utils.hpp"
#include "Polyline_Sweep_Cylinder.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include "Unit_Table.hpp"


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

    vector<vector<int>> jb_value_list;

    for (int i = 0; i < card_count;i++) {
        vector<int> list;
        for (int t = 0; t < 8;t++) {
            list.push_back(buffer[6+i*8+t]);

        }
        jb_value_list.push_back(list);

    }

    vector<vector<vector<int>>> actions;
    int base = 6+8* card_count;

    vector<int> card_data_unpack = bytes_unpack(buffer,base, base + Z*S*card_count/8);

    int stride_S = card_count * Z;
    int stride_cd = Z;


    for (int y = 0; y < card_count; y++) {
        actions.push_back(vector<vector<int>>());
    }

    for (int y = 0; y < S;y++) {

        for (int c = 0; c < card_count;c++) {

            vector<int> line;
            for (int i = 0; i < Z;i++) {
                line.push_back(card_data_unpack[y* stride_S + c* stride_cd + i]);
            }
            actions[c].push_back(line);
        }


    }





    delete[](buffer);

    return FZ_Data(version, Z, S,card_count, jb_value_list, actions);
}

void Draw_Line(const Unit_Table& unit_table,
    int card_id,const float stretch,float bed_distance,
    vector<vector<v3_f>>& curve_list,
    const List3<bool>& bed_0_right_side_linked, 
    const List3<bool>& bed_1_right_side_linked) {


    int width = unit_table.width;
    int height = unit_table.height;

    //
    curve_list.clear();

    curve_list.reserve(width);

    for (int x = 0; x < width; x++) {
        curve_list.push_back(vector<v3_f>{});
    }


    auto draw_line_item = [&curve_list,&unit_table,
        &height,&width,
        stretch, card_id, bed_distance,
        &bed_0_right_side_linked,&bed_1_right_side_linked](int x)->void {

        //std::cout << su::fmt("\r绘制线圈 {}/{}       ", { x ,width });

        vector<v3_f> curve_point_list;
        curve_point_list.reserve(height);

        for (int y = 0; y < height; y++) {

            const Act_Unit& unit = unit_table.table[card_id][x][y];

            int bed_0_front = unit.bed_0_front  ;
            int bed_0_back = unit.bed_0_back    ;
            int bed_1_front = unit.bed_1_front  ;
            int bed_1_back = unit.bed_1_back    ;

            v3_f center = { x + f(bed_0_front + bed_0_back) / 2, f(y) / f(1.5), 0.0 };
            v3_f center_1 = { center.x,center.y,center.z+ bed_distance };

            int b_0_curve_poc_x = x + min(bed_0_front, bed_0_back);
            int b_0_curve_poc_y = y;

            if (x!=0 && x!= width-1 && y<height-2) {

                bool right_link = bed_0_right_side_linked.cGet(card_id, b_0_curve_poc_x, b_0_curve_poc_y+2);//网上偏移两个格
                bool left_link = bed_0_right_side_linked.cGet(card_id, b_0_curve_poc_x-1, b_0_curve_poc_y+2);

                if (left_link && !right_link) {
                    center = center + v3_f{-0.2,0,0};
                }
                else if (!left_link && right_link) {
                    center = center + v3_f{ 0.2,0,0 };
                }


            }




            vector<v3_f> unit_points;
            if (bed_0_front < bed_0_back) {
                unit_points = Get_Curve(center, n_in, mid_v, mid_v, stretch);
            }
            else if (bed_0_back < bed_0_front) {
                unit_points = Get_Curve(center, p_in, mid_v, mid_v, stretch);
            }
            else {
                unit_points = {};
            }
            curve_point_list.insert(curve_point_list.end(), unit_points.begin(), unit_points.end());

            if (bed_1_front < bed_1_back) {
                unit_points = Get_Curve(center_1, n_in, mid_v, mid_v, stretch);
            }
            else if (bed_1_back < bed_1_front) {
                unit_points = Get_Curve(center_1, p_in, mid_v, mid_v, stretch);
            }
            else {
                unit_points = {};
            }

            curve_point_list.insert(curve_point_list.end(), unit_points.begin(), unit_points.end());


        }

        curve_list[x] = curve_point_list;

    };

     for (int x = 0; x < width;x++) {
         // std::thread(draw_line_item,x));
         draw_line_item(x);
     }

    //vector<std::thread> thread_list;
    //for (int x = 0; x < width;x++) {
    //    thread_list.push_back(std::thread(draw_line_item,x));
    //}
    //for (int x = 0; x < width; x++) {
    //    thread_list[x].join();
    //}

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

    void Generate_Curve(string fz_path,float sweep_width,int segments,float stretch,const vector<vector<vector<int>>> jb_value_list_input, 
                        vector<vector<vector<int>>>& jb_value_list_res,
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
            std::cout << "stretch	\t\t: "			<< stretch << endl;
        }
        catch (...) {
            std::cout<<"input wrong!" << endl;
            return;
        }

        FZ_Data fz_data = read_fz(fz_path);

        int fz_width = fz_data.Z;
        int fz_height = fz_data.S ;
        int fz_card_count = fz_data.card_count;
        vector<vector<vector<int>>> jb_value_format =  fz_data.Get_jb_value_List();// 贾卡号-奇偶列-数码序列


        std::cout <<" odd and even column code" << endl;
        
        for (int c = 0; c < 3;c++) {
            for (int o = 0; o < 2;o++) {
                if (jb_value_list_input[c][o][0]==-1) {

                }
                else {
                    jb_value_format[c][o] = jb_value_list_input[c][o];
                }
            }
        }
        for (int c = 0; c < 3; c++) {
            for (int o = 0; o < 2; o++) {
                for (int i = 0; i < 8;i++) {
                    cout << jb_value_format[c][o][i]<<" ";
                }
                cout << "\n";
            }
            cout << "\n";
        }
        jb_value_list_res = jb_value_format;


        Unit_Table unit_table = { fz_width, fz_height,fz_card_count, jb_value_format };

        unit_table.Apply_Actions(fz_data.actions);
        unit_table.Test_Linked();

        vector<vector<v3_f>> curve_list;

        for (int c = 0; c < fz_card_count;c++) {
            vector<vector<v3_f>> curve_list_tmp;

            Draw_Line(unit_table, c, stretch, 10, curve_list_tmp, 
                unit_table.bed_0_right_side_linked, unit_table.bed_1_right_side_linked);

            curve_list.insert(curve_list.end(), curve_list_tmp.begin(), curve_list_tmp.end());
        }
        vector<vector<v3_f>> curve_smoothed_list;

        for (const vector<v3_f>& curve: curve_list) {

            vector<v3_f> smoothed = Curve_Smooth(curve);

            curve_smoothed_list.push_back(smoothed);

        }
        /*

        ofstream fout("t.obj",ios::out|ios::binary);
        string txt;

        for (vector<v3_f>& curve: curve_smoothed_list) {
            for (v3_f& p: curve) {
                txt += su::fmt("v {} {} {}\n", {p.x,p.y,p.z});
            }
        }
        int index = 1;
        for (vector<v3_f>& curve : curve_smoothed_list) {
            txt += su::fmt("l ", { });
            for (v3_f& p : curve) {
                txt += su::fmt("{} ", { index });

                index++;
            }

            txt += "\n";
        }
        fout.write(txt.c_str(),txt.size());
        
        return;
        /**/
        vertices_result_list.clear();
        faces_result_list.clear();
        normal_result_list.clear();

        
        for (int i = 0; i < curve_smoothed_list.size(); i++)
        {
            cout << su::fmt("\r{}/{}       ", {i,curve_smoothed_list.size() });

            vector<v3_f> vertices_result;
            vector<v3<uint>> faces_result;
            vector<v3_f> normal_result;

            Sweep(i,sweep_width, segments, curve_smoothed_list[i], vertices_result, normal_result, faces_result);
            vertices_result_list.push_back(vertices_result);
            faces_result_list.push_back(faces_result);
            normal_result_list.push_back(normal_result);

        }
        

        cout<<"DONE"<<endl;
    }


private:

public:









};






