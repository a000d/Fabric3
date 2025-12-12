#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"utils.hpp"
#include<math.h>

void Polyline_Sweep_Cylinder(const vector<v3_f>& points, const float radius, const int segments,
                             vector<v3_f>& vertices,
                             vector<v3<unsigned int>>& faces) {

    vector<v3_f> circle_template;

    for (int i = 0; i < segments; i++) {
        float angle = 2 * PI * i / segments;
        float x = cos(angle);
        float y = sin(angle);
        circle_template.push_back({ x,y,0 });
    }

    //为每个路径点生成圆环
    for (int i = 0; i < points.size(); i++) {

        const v3_f current_pos = points[i];

        v3_f direction;
        //计算当前点的方向
        if (i == 0) {
            v3_f next_pos = points[i + 1];
            direction = next_pos - current_pos;

        }
        else if (i == points.size() - 1) {
            v3_f prev_pos = points[i - 1];
            direction = current_pos - prev_pos;

        }
        else {
            v3_f prev_pos = points[i - 1];
            v3_f next_pos = points[i + 1];
            direction = (next_pos - prev_pos) / 2;
        }

        float direction_length = direction.length();

        v3_f z_axis;
        v3_f x_axis;
        v3_f y_axis;

        if (direction_length < 1e-5) {
            z_axis = { 0,0,1 };
        }
        else {
            z_axis = direction / direction_length;
        }
        if (abs(z_axis.z) > 0.9) {
            x_axis = { 1,0,0 };
        }
        else {
            x_axis = cross({ 0,0,1 }, z_axis);
        }
        x_axis = x_axis / x_axis.length();
        y_axis = cross(z_axis, x_axis);

        for (const v3_f& circle_point : circle_template) {
            const v3_f& local_point = circle_point;
            v3_f world_point = (x_axis * local_point.x + y_axis * local_point.y) * radius + current_pos;
            vertices.push_back(world_point);

            //cout << world_point.x << world_point.y << world_point.z << endl;
        }
    }
    for (int i = 0; i < points.size() - 1; i++) {
        for (int j = 0; j < segments; j++) {

            int current_ring_start = i * segments;
            int next_ring_start = (i + 1) * segments;

            unsigned int v1 = current_ring_start + j + 1;
            unsigned int v2 = current_ring_start + (j + 1) % segments + 1;
            unsigned int v3 = next_ring_start + (j + 1) % segments + 1;
            unsigned int v4 = next_ring_start + j + 1;

            v1 = v1-1;
            v2 = v2-1;
            v3 = v3-1;
            v4 = v4-1;

            faces.push_back({ v1, v2, v3 });
            faces.push_back({ v1, v3, v4 });

        }
    }

}
