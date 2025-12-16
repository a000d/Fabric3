#include <iostream>
#include "Fabric_Tool.hpp"


int main()
{

    Fabric_Tool fabric_tool;

    vector<vector<v3_f>> vertices_result_list;
    vector<vector<v3<uint>>> faces_result_list;
    vector<vector<v3_f>> normal_result_list;

    fabric_tool.Generate_Curve("C:/Users/z/Desktop/fz_test_3/resource/fabric/PCTCJ6240F-00A-37.fz",
        0.05,3,"-1",1, 
        vertices_result_list, faces_result_list, normal_result_list);







}
