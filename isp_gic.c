//
// Created by ly on 2022/4/03.
//
//green imbalance correction
#include "isp_gic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

void reshape(uint16* Input,uint16**Output,uint16 img_width,uint16 img_height){

    uint16 m = img_width;
    uint16 n = img_height;
    for(uint16 v = 0;v <n;v++){
        for(uint16 h=0;h<m;h++){
            Output[v][h]=Input[v*m+h];
        }
    }
    return;
}
void weighted_filter3x3(uint16*value_in_kernel,const uint8 *weight_table,uint8 *w_lut,uint8 threshold){

    uint16 diff_val[5];
    diff_val[0] = 0;
    diff_val[1] = abs(value_in_kernel[1]-value_in_kernel[0]);
    diff_val[2] = abs(value_in_kernel[2]-value_in_kernel[0]);
    diff_val[3] = abs(value_in_kernel[3]-value_in_kernel[0]);
    diff_val[4] = abs(value_in_kernel[4]-value_in_kernel[0]);
    //uint16 thresh = 17*value_in_kernel[0] >>7;
    //printf("LINE-%d,thresh:%d,diff_val[1]:%d, diff_val[2]:%d,diff_val[3]:%d,diff_val[4]:%d\n",__LINE__,thresh,diff_val[1],diff_val[2],diff_val[3],diff_val[4]);
    for(uint8 i=0;i<5;i++){
        uint16 temp_threshold = threshold*value_in_kernel[0] >>7;

        if(diff_val[i]>=temp_threshold){
            w_lut[i] = 0;
        }
        else{
            uint8 temp_j = (uint8)((float)diff_val[i]/value_in_kernel[0])<<7;
            if(temp_j >=17){
                temp_j = temp_j/17;
            }
            //printf("LINE-%d,temp_j:%d \n",__LINE__,temp_j);
            w_lut[i] = weight_table[temp_j];
        }
        //printf("LINE-%d,w_lut:%d \n",__LINE__,w_lut[i]);
    }
    //printf("LINE-%d \n",__LINE__);
    return;
}
void core_gic(uint16 **input,const raw_info raw_info,uint8 str,uint8 thresh){

    uint16 in_width = raw_info.u16ImgWidth;
    uint16 in_height =raw_info.u16ImgHeight;
    uint8  BP = raw_info.u8BayerPattern;
    uint8 start_x,start_y;
    uint16 val[5] ={0};
    uint16 w_lut[5] ={0};
    const uint8 weight_table[]={
            255,240,225,210,195,180,165,150,
            135,120,105, 90, 75 ,60, 45, 30,
            15};
    if(BPRG == BP || BPBG == BP){
        start_x = 2;
        start_y = 2;
    }
    else {
        start_x = 1;
        start_y = 1;
    }
    for(uint16 y = start_y;(y+1)< in_height;y=y+2){
        for(uint16 x= start_x;(x+1)< in_width;x=x+2){
            val[0] = input[y][x];
            val[1] = input[y-1][x-1];
            val[2] = input[y-1][x+1];
            val[3] = input[y+1][x-1];
            val[4] = input[y+1][x+1];
            weighted_filter3x3(val,weight_table,w_lut,thresh);
            input[y][x] = (input[y][x]<<14 + str*(w_lut[1]*val[1]+w_lut[2]*val[2]+w_lut[3]*val[3]+w_lut[4]*val[4]))/(1<<14+str*(w_lut[1]+w_lut[2]+w_lut[3]+w_lut[4]));
        }

    }
}
int main(int argc,char**argv){

//    static uint16 gamma_table[] = {
//        #include "gamma_table.h"
//    };

    const raw_info raw_info = {0,0,1920,1080,12,BPRG};
    uint16 raw_width = raw_info.u16ImgWidth;
    uint16 raw_height = raw_info.u16ImgHeight;
    char *raw_file = "D:\\all_isp\\test_img\\lab_1920x1080_12bits_RGGB_Linear.raw";
    char *before_gic = "D:\\leetcode_project\\before_gic_img.bmp";
    char *after_gic = "D:\\leetcode_project\\after_gic_img.bmp";

    uint16* BayerImg = (uint16*)malloc(raw_width * raw_height * sizeof(uint16));
    if( NULL == BayerImg ){
        printf("BayerImg malloc fail!!!\n");
    }
    uint16** m_BayerImg = (uint16**)malloc(raw_height*sizeof(uint16*));
    for(uint16 v=0;v < raw_height;v++){
        m_BayerImg[v] = (uint16*) malloc(raw_width*sizeof(uint16));
    }

    read_BayerImg(raw_file,raw_height,raw_width,BayerImg);
    //print_raw_to_txt(BayerImg,1920,1080,save_raw);
    reshape(BayerImg,m_BayerImg,raw_width,raw_height);

    singleChannel2BMP(m_BayerImg,raw_width,raw_height,before_gic);
    
    uint8 gic_str = 128;//gic strength,[0,255]
    uint8 gic_threshold = 17;//gic threshold,[0,127],gr/gb diff threshold
    core_gic(m_BayerImg,raw_info,gic_str,gic_threshold);
    singleChannel2BMP(m_BayerImg,raw_width,raw_height,after_gic);


    for(uint16 p=0; p <raw_height;p++){
        free(m_BayerImg[p]);
    }
    free(m_BayerImg);


    free(BayerImg);
    return 0;

}





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */