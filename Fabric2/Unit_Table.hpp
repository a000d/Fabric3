#pragma once

#include "utils.hpp"

struct Act_Unit {
public:
    int x;
    int y;

    int bed_0_front;
    int bed_0_back;

    int bed_1_front;
    int bed_1_back;


public:
    Act_Unit(int x, int y, int bed_0_front, int bed_0_back, int bed_1_front, int bed_1_back)
        :x(x), y(y),
        bed_0_front(bed_0_front), bed_0_back(bed_0_back),
        bed_1_front(bed_1_front), bed_1_back(bed_1_back)
    {


    }
    v3<float> Get_Center() const {
        return { (float)(x - (bed_0_front + bed_0_back) / 2), (float)(y / 1.5), 0.0 };

    }


};


class Unit_Table {
public:
    int width;
    int height;
    int card_count;
    vector<vector<vector<int>>> jb_value_list;// 贾卡号-奇偶列-数码序列
    vector<vector<vector<Act_Unit>>> table;
    List3<bool> bed_0_right_side_linked;//单元右侧是否存在线牵拉
    List3<bool> bed_1_right_side_linked;

public:
    Unit_Table(int width, int height, int card_count, const vector<vector<vector<int>>>& jb_value_list) {

        this->width = width;
        this->height = height / 4;
        this->jb_value_list = jb_value_list;
        this->card_count = card_count;

        for (int card_id = 0; card_id < card_count; card_id++) {
            table.push_back(vector<vector<Act_Unit>>());
        }

        for (int card_id = 0; card_id < card_count; card_id++) {

            const vector<vector<int>>& card_jb_value = jb_value_list[card_id];

            for (int x = 0; x < width; x++) {
                vector<Act_Unit> tmp_list;

                for (int y = 0; y < height / 2; y++) {



                    if (x % 2 == 1) {

                        tmp_list.push_back(Act_Unit(x, y, card_jb_value[0][0], card_jb_value[0][1], card_jb_value[0][2], card_jb_value[0][3]));
                        tmp_list.push_back(Act_Unit(x, y, card_jb_value[0][4], card_jb_value[0][5], card_jb_value[0][6], card_jb_value[0][7]));
                    }
                    else {
                        tmp_list.push_back(Act_Unit(x, y, card_jb_value[1][0], card_jb_value[1][1], card_jb_value[1][2], card_jb_value[1][3]));
                        tmp_list.push_back(Act_Unit(x, y, card_jb_value[1][4], card_jb_value[1][5], card_jb_value[1][6], card_jb_value[1][7]));
                    }


                }

                table[card_id].push_back(tmp_list);
            }
        }

    }

    void Apply_Actions(const vector<vector<vector<int>>>& actions) {

        for (int card_id = 0; card_id < card_count; card_id++) {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {


                    table[card_id][x][y].bed_0_front += actions[card_id][y * 4 + 0][x];
                    table[card_id][x][y].bed_0_back += actions[card_id][y * 4 + 1][x];

                    table[card_id][x][y].bed_1_front += actions[card_id][y * 4 + 2][x];
                    table[card_id][x][y].bed_1_back += actions[card_id][y * 4 + 3][x];
                }

            }
        }

    }


    void Test_Linked() {

        bed_0_right_side_linked.Init(card_count, width + 8, height);
        bed_1_right_side_linked.Init(card_count, width + 8, height);


        for (int card_id = 0; card_id < card_count; card_id++) {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height - 1; y++) {

                    const Act_Unit& unit_current = table[card_id][x][y];
                    const Act_Unit& unit_up = table[card_id][x][y + 1];

                    //bed_0
                    {
                        int bed_0_curr_front = unit_current.bed_0_front;
                        int bed_0_curr_back = unit_current.bed_0_back;
                        int min_curr = min(bed_0_curr_front, bed_0_curr_back);

                        int bed_0_up_front = unit_up.bed_0_front;
                        int bed_0_up_back = unit_up.bed_0_back;
                        int min_up = min(bed_0_up_front, bed_0_up_back);

                        if (min_up > min_curr) {
                            for (int i = 0; i < min_up - min_curr; i++) {
                                //bed_0_right_side_linked[card_id][x + min_curr + i][y] = true;
                                //bed_0_right_side_linked[card_id][x + min_curr + i][y + 1] = true;
                                bed_0_right_side_linked.Get(card_id, x + min_curr + i, y) = true;
                                bed_0_right_side_linked.Get(card_id, x + min_curr + i, y + 1) = true;

                            }


                        }
                        else if (min_up < min_curr) {
                            for (int i = 0; i < abs(min_up - min_curr); i++) {
                                //bed_0_right_side_linked[card_id][x + min_curr - 1-i][y] = true;
                                //bed_0_right_side_linked[card_id][x + min_curr - 1-i][y + 1] = true;

                                bed_0_right_side_linked.Get(card_id, x + min_curr - i - 1, y) = true;
                                bed_0_right_side_linked.Get(card_id, x + min_curr - i - 1, y + 1) = true;
                            }

                        }
                    }
                    //bed_1
                    {
                        int bed_1_curr_front = unit_current.bed_1_front;
                        int bed_1_curr_back = unit_current.bed_1_back;
                        int min_curr = min(bed_1_curr_front, bed_1_curr_back);

                        int bed_1_up_front = unit_up.bed_1_front;
                        int bed_1_up_back = unit_up.bed_1_back;
                        int min_up = min(bed_1_up_front, bed_1_up_back);

                        if (min_up > min_curr) {
                            for (int i = 0; i < min_up - min_curr; i++) {
                                //bed_0_right_side_linked[card_id][x + min_curr + i][y] = true;
                                //bed_0_right_side_linked[card_id][x + min_curr + i][y + 1] = true;
                                bed_1_right_side_linked.Get(card_id, x + min_curr + i, y) = true;
                                bed_1_right_side_linked.Get(card_id, x + min_curr + i, y + 1) = true;

                            }


                        }
                        else if (min_up < min_curr) {
                            for (int i = 0; i < abs(min_up - min_curr); i++) {
                                //bed_0_right_side_linked[card_id][x + min_curr - 1-i][y] = true;
                                //bed_0_right_side_linked[card_id][x + min_curr - 1-i][y + 1] = true;

                                bed_1_right_side_linked.Get(card_id, x + min_curr - i - 1, y) = true;
                                bed_1_right_side_linked.Get(card_id, x + min_curr - i - 1, y + 1) = true;
                            }

                        }
                    }


                }
            }
        }

        ofstream fout("occupy.txt", ios::binary | ios::out);
        string txt;
        for (int x = 0; x < width + 2; x++) {
            for (int y = 0; y < height - 1; y++) {

                if (bed_0_right_side_linked.Get(0, x, y)) {
                    txt += "1";
                }
                else {
                    txt += " ";
                }


            }
            txt += "\n";
        }
        fout.write(txt.c_str(), txt.size());








    }



};




