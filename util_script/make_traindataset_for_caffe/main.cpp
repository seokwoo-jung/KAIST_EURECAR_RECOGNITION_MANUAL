#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <ctime>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <error.h>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <sstream>

using namespace std;

int main(int argc,char* argv[])
{
    if ((argc == 1) || (argc == 2))
    {
        cout << "Useage : ./make_traindataset_for_caffe [IMG_FOLDER_PATH] [LABEL_FOLDER_PATH]" << endl;
    }
    else if(argc == 3)
    {
        string dir_file;
        DIR *dir;
        struct dirent *ent;

        string img_folder_path(argv[1]);
        string label_folder_path(argv[2]);
        cout << "img folder path : " << img_folder_path << endl;
        cout << "label folder path : " << label_folder_path << endl;

        vector<string> img_folder_path_list;
        vector<string> label_folder_path_list;

        dir = opendir(img_folder_path.c_str());

        if (dir != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                dir_file = ent->d_name;
                if((dir_file == ".") || (dir_file == ".." ))
                    continue;

                img_folder_path_list.push_back(dir_file);
            }
        }
        else
        {
            cout << "Cannot find Img folder path" << endl;
        }

        dir = opendir(label_folder_path.c_str());

        if (dir != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                dir_file = ent->d_name;
                if((dir_file == ".") || (dir_file == ".." ))
                    continue;

                label_folder_path_list.push_back(dir_file);
            }
        }
        else
        {
            cout << "Cannot find label folder path" << endl;
        }

        // Shuffle
        srand(unsigned(std::time(0)));

        std::random_shuffle(img_folder_path_list.begin(), img_folder_path_list.end());


        ofstream ofile;
        ofile.open("trainval.txt");
        for(vector<string>::iterator it = img_folder_path_list.begin(); it < img_folder_path_list.end();++it)
        {
            string img_file_name = (*it);
            // Find File Name
            string file_name;
            istringstream iss(img_file_name);
            string token;
            string img_type;
            uint count = 0;
            while(getline(iss,token,'.'))
            {
                if(count == 0)
                    file_name = token;
                else
                    img_type = token;

                count++;
            }


            string corresp_label_path = label_folder_path + "/" + file_name + ".xml";
            string corresp_img_path = img_folder_path + "/" + file_name + "." + img_type;

            // Check existence of corresponding label file
            if(access(corresp_label_path.c_str(), F_OK) == 0)
            {
                string line_str = corresp_img_path + " " + corresp_label_path;
                cout << corresp_img_path + " " + corresp_label_path << endl;
                ofile << line_str << endl;
            }
            else
            {
//                cout << "Label file path : " << corresp_label_path << " is not existed" << endl;
            }
        }

        ofile.close();
    }

    return 0;
}


