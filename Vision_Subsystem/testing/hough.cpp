#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

int image [240][320];
typedef struct{
    int x;
    int y;
} edges_coords_struct;
typedef struct {
    int r;
    int x0;
    int y0;
}point;

typedef struct {
    int r;
    int x_mid;
    int y_mid;
} circle;

typedef struct {
    int a;
    int b;
    int r;
    int acc_val;
} accum;

edges_coords_struct edges_coords[320*240];
int main() {
    std::ifstream fin;
    fin.open("greyscale.csv");
    int row = 0, col = 0, edge_idx = 0;
    std::string line;
    std::stringstream linestream;
    std::string cell;
    while (std::getline(fin, line)){
        linestream.clear();
        linestream.str(line);
        while(std::getline(linestream ,cell, ',')){
            // image[row][col] = std::stoi(cell);
            int pixel = std::stoi(cell);
            // std::cout<<pixel<<"\n";
        
            if (pixel == 255) {
                edges_coords_struct new_edge = {row, col}; 
                edges_coords[edge_idx] = new_edge;
                edge_idx += 1;
            }
            col += 1;
        }
        col = 0;
        row+= 1;

    }
    for (int i = 0; i < 20; i++){
        std::cout<<edges_coords[i].x<<" "<<points_list[i].x0<<" "<< points_list[i].y0<<"\n";
    }
    int rmin = 10;
    int rmax = 60;
    int steps = 10;
    float threshold = 0.6;
    
    point points_list [204];
    int point_idx = 0;
    for (int r = rmin; r < rmax; r++){
        point new_point = {r, 0, r};
        points_list[point_idx]= new_point;
        point_idx += 1;
        point new_point1 = {r, r, 0};
        points_list[point_idx]=  new_point1;
        point_idx += 1;
        point new_point2 = {r, 0, -r};
        points_list[point_idx]=  new_point2;
        point_idx += 1;
        point new_point3 = {r, -r, 0};
        points_list[point_idx]= new_point3;
        point_idx += 1;
    }
    for (int i = 0; i < 20; i++){
        std::cout<<points_list[i].r<<" "<<points_list[i].x0<<" "<< points_list[i].y0<<"\n";
    }
    std::cout<<point_idx<<" "<<edge_idx<<" "<< rmax-rmin<<"\n";
    int acc_list [240][320][rmax-rmin]  = {0};
    for (int i = 0; i < edge_idx; i++){
        for (int j = 0; j < point_idx; j++){
            int a = edges_coords[i].x - points_list[j].x0;
            int b = edges_coords[i].y - points_list[j].y0;
            int rad = points_list[j].r;
            std::cout<<a<<" "<<b<<" "<<rad<<"\n";
            acc_list[a][b][rad-rmin] += 1 ; 
        }
    }
    for (int i = 0; i < 240; i++){
        for (int j = 0; j < 320; j++){
            for (int r = 0; r < rmax-rmin; r++){
                std::cout<<acc_list[i][j][r]<<"\n";
                if (acc_list[i][j][r] > 1){
                    std::cout<<"Circle: x: "<<i<<", y: "<<j<<", r: "<<r<<"\n";
                }
            }
        }
    }
}