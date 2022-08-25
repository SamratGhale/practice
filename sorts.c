#include "stdio.h"
#include "stdlib.h"

#define ArrayCount(a)(sizeof(a)/sizeof(a[0]));

int * merge_sort(int * arr, int n){
  if(n <= 1){
    return arr;
  }else if(n == 2){
    if(arr[0] > arr[1]){
      int temp = arr[0]; 
      arr[0] = arr[1];
      arr[1] = temp;
      return arr;
    }else{
      return arr;
    }
  }
  int m = (int)(n/2);
  
  int * arr1 = (int*)malloc(m * sizeof(int));
  memcpy(arr1, arr, m * sizeof(int));
  
  int * arr2 = (int*)malloc((n-m) * sizeof(int));
  memcpy(arr2, arr + m, (n-m) * sizeof(int));
  
  int * newarr1 = merge_sort(arr1, m);
  int * newarr2 = merge_sort(arr2, (n-m));
  
  while(true){
    
  }
  
  return arr;
}
  

int main(char argv, char * args[]){
  int arr[] = {1, 3, 2, 9, 51, 6};
  
  
  
  int * arr2 = (int*)malloc(3 * sizeof(int));
  memcpy(arr2, arr + 2, 3  * sizeof(int));
  
  for(int i = 0; i < 3; i++){
    printf("newarr[%d] = %d\n", i, arr2[i]);
  }
}