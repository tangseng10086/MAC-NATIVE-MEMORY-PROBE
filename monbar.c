#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
// macos核心Mach内核接口
#include<mach/mach.h>
// 内存统计的相关定义
#include<mach/vm_statistics.h>
// 这个用来检测总到内存情况
void check_memory()
{
    // 先创建内存输入容器
    mach_msg_type_number_t count=HOST_VM_INFO_COUNT;
    vm_statistics_data_t vm_status;
    // 向内存发送请求
    if (host_statistics(mach_host_self(),HOST_VM_INFO,(host_info_t)&vm_status,&count)!=KERN_SUCCESS) {
        fprintf(stderr,"无法获取系统内存信息\n");
        return;
    }
    // 这里为了减小内存占用，采用直接给容器的地址发送数据而不是复制容器
    // 不需要再写else分支，程序会执行if内部的语句
    // 格式化总的内存数据
    long page_size=sysconf(_SC_PAGESIZE);
    long long free_mem=(long long)vm_status.free_count*page_size/(1024*1024);
    long long active_mem=(long long)vm_status.active_count*page_size/(1024*1024);
    long long inactive_mem=(long long)vm_status.inactive_count*page_size/(1024*1024);
    long long wired_mem=(long long)vm_status.wire_count*page_size/(1024*1024);
    long long used_mem=active_mem+inactive_mem+wired_mem;
    // 计算出总的内存占用
    // 格式化app本身的内存数据和变量初始化
    struct mach_task_basic_info task_info_data;
    mach_msg_type_number_t task_info_count=MACH_TASK_BASIC_INFO_COUNT;
    double my_app_mem=0.0;
    // 向内存发送请求
    if (task_info(mach_task_self(),MACH_TASK_BASIC_INFO,(task_info_t)&task_info_data,&task_info_count)==KERN_SUCCESS) {
        my_app_mem=(double)task_info_data.resident_size/(1024*1024.0);
        // printf("1");
    }
    double usage_percent=(double)used_mem/8192.0*100;
    // 这里根据swiftbar的格式做了特定的输出
    // 展示数据(利用展示数据的颜色来设置预警)
    if (usage_percent>90.0) {
        // 利用红色展示内存告急的数据
        printf("RAM:%.2lf|color=red\n",usage_percent);
    } else{
        // 正常展示数据
        printf("RAM:%.2lf%%\n",usage_percent);
        // 设置下拉菜单
        printf("---\n");
        // 设置具体数据
        printf("已用:%lldMB",used_mem);
        printf("空闲:%lldMB",free_mem);
        printf("探针占用:%.2lfMB",my_app_mem);
    }
}
int main()
{
    printf("轻量级内存监视器启动\n");
    // 这里根据swiftbar的文件特点没有加循环
    // 输出本机相关数据
    // printf("Page Size:%ld字节",sysconf(_SC_PAGE_SIZE))
    check_memory();
    return 0;
}