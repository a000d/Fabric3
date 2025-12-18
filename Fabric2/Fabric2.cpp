#include <iostream>
#include "Fabric_Tool.hpp"


int main()
{

    Fabric_Tool fabric_tool;

    vector<vector<v3_f>> vertices_result_list;
    vector<vector<v3<uint>>> faces_result_list;
    vector<vector<v3_f>> normal_result_list;
    vector<vector<vector<int>>> jb_value_list_res;// 贾卡号-奇偶列-数码

    vector<vector<vector<int>>> jb_value_list_input = { 
        {{1,0,1,1,1,2,1,1},{1,2,1,1,1,0,1,1}},
        {{-1,-1,-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1,-1,-1}},
        {{-1,-1,-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1,-1,-1}} 
    };

    fabric_tool.Generate_Curve("C:/Users/z/Desktop/fz_test_3/resource/fabric/17709.fz",
        0.05,3,1, jb_value_list_input, jb_value_list_res,
        vertices_result_list, faces_result_list, normal_result_list);



    cout << 1;



}
