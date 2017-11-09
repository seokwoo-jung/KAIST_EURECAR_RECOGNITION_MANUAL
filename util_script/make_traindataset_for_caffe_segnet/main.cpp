#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <ctime>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <error.h>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

int main(int argc,char* argv[])
{
    if (argc != 6)
    {
        cout << "Useage : ./make_traindataset_for_caffe_segnet [IMG_FOLDER_PATH] [LABEL_COLOR_FOLDER_PATH] [LABEL_COLOR_INDEX_FILE_PATH] [TARGET_IMG_WIDTH] [TARGET_IMG_HEIGHT]" << endl;
    }

    /* PARSING LABEL_COLOR_INDEX_FILE *******************
     *
     * */
    string label_rgb_index_file_path(argv[3]);
    ifstream label_rgb_index_file(label_rgb_index_file_path.c_str());
    vector<pair<cv::Point3d,string> > label_rgb_data_list;
    string line_str;
    while(getline(label_rgb_index_file,line_str))
    {
        istringstream iss(line_str);

        pair<cv::Point3d,string> label_rgb_data;
        string parsed_str;

        uint index = 0;
        while(getline(iss,parsed_str,','))
        {
            switch(index)
            {
            case 0:
                label_rgb_data.first.x = std::atoi(parsed_str.c_str());
                break;
            case 1:
                label_rgb_data.first.y = std::atoi(parsed_str.c_str());
                break;
            case 2:
                label_rgb_data.first.z = std::atoi(parsed_str.c_str());
                break;
            case 3:
                label_rgb_data.second = parsed_str;
                break;
            }
            index++;
        }
        label_rgb_data_list.push_back(label_rgb_data);
    }
    /******************************************************/


    /* LOAD IMG & LABEL_COLOR_FILES ***********************
     *
     * */
    string img_folder_path(argv[1]);
    string label_color_folder_path(argv[2]);

    string dir_file;
    DIR *dir;
    struct dirent *ent;

    vector<string> img_file_path_list;
    dir = opendir(img_folder_path.c_str());
    if( dir != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            dir_file = ent->d_name;
            if((dir_file == ".") || (dir_file == ".." ))
                continue;

            string img_file_path = img_folder_path + "/" + dir_file;
            img_file_path_list.push_back(img_file_path);
        }
    }

    std::sort(img_file_path_list.begin(),img_file_path_list.end());


    vector<string> label_color_file_path_list;
    dir = opendir(label_color_folder_path.c_str());
    if( dir != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            dir_file = ent->d_name;
            if((dir_file == ".") || (dir_file == ".." ))
                continue;

            string label_color_file_path = label_color_folder_path + "/" + dir_file;
            label_color_file_path_list.push_back(label_color_file_path);
        }
    }
    /******************************************************/


    /* CONVERT TO CAFFE DATASET ***************************
     *
     * */
    char cwd[1024];
    getcwd(cwd,sizeof(cwd));
    string cwd_str = string(cwd);

    cv::Size target_size;
    target_size.width = atoi(argv[4]);
    target_size.height = atoi(argv[5]);

    mkdir("caffe_img",0700);
    mkdir("caffe_label",0700);

    ofstream ofile("trainval.txt");

    for(uint img_index = 0;img_index < img_file_path_list.size();img_index++)
    {
        string img_file_path = img_file_path_list.at(img_index);

        istringstream iss(img_file_path);

        string parsed_str;
        while(getline(iss,parsed_str,'/'))
        {

        }
        string img_file_name_only = parsed_str;

        istringstream iss2(parsed_str);

        string img_file_name;
        while(getline(iss2,img_file_name,'.'))
        {
            break;
        }

        string label_color_file_path = label_color_folder_path + "/" + img_file_name + ".png";
        string label_color_file_name = img_file_name + ".png";

        cv::Mat ori_img = cv::imread(img_file_path);

        bool find_both = false;
        cv::Mat label_color_img;
        if(!ori_img.empty())
        {
            label_color_img = cv::imread(label_color_file_path);
            if(label_color_img.empty())
            {
                label_color_file_path = label_color_folder_path + "/" + img_file_name + ".jpg";
                label_color_file_name = img_file_name + ".jpg";
                label_color_img = cv::imread(label_color_file_path);
                if(!label_color_img.empty())
                {
                    find_both = true;
                }
            }
            else
            {
                find_both = true;
            }
        }

        if(find_both)
        {
            cv::Mat ori_img_resized;
            cv::Mat label_color_img_resized;

            cv::resize(ori_img,ori_img_resized,target_size);
            cv::resize(label_color_img,label_color_img_resized,target_size);

            cv::Mat label_img = cv::Mat::zeros(target_size,CV_8UC1) ;

            for(uint x=0;x < label_color_img_resized.cols;x++)
            {
                for(uint y=0;y < label_color_img_resized.rows;y++)
                {
                    uint r_value = (uint)label_color_img_resized.at<cv::Vec3b>(y,x)[2];
                    uint g_value = (uint)label_color_img_resized.at<cv::Vec3b>(y,x)[1];
                    uint b_value = (uint)label_color_img_resized.at<cv::Vec3b>(y,x)[0];


                    bool find_target_class = false;
                    uint target_class = 0;
                    for(uint class_index = 1; class_index < label_rgb_data_list.size();class_index++)
                    {
                        if((label_rgb_data_list.at(class_index).first.x == r_value) && (label_rgb_data_list.at(class_index).first.y == g_value) && (label_rgb_data_list.at(class_index).first.z == b_value))
                        {
                            find_target_class = true;
                            target_class = class_index;
                        }
                    }

                    if(!find_target_class)
                    {
                        target_class = 0;
                    }

                    label_img.at<uchar>(y,x) = target_class;
                }
            }

            string target_img_path = cwd_str + "/caffe_img/" + img_file_name_only;
            string target_label_path = cwd_str + "/caffe_label/" + label_color_file_name;

            cout << target_img_path << endl;
            cout << target_label_path << endl;

            string ofile_line = target_img_path + " " + target_label_path;

            ofile << ofile_line << endl;

            cv::imwrite(target_img_path,ori_img_resized);
            cv::imwrite(target_label_path,label_img);

        }
    }
    ofile.close();
    /******************************************************/


    return 0;
}
