#include "gui.h"

struct Node{
  u32 Color;
  Node * next;

  Node(u32);
  void set_last(u32 );
  void remove_last();
};

Node::Node(u32 Color){
  this->Color = Color;
  this->next  = NULL;
}

void Node::set_last(u32 Color){
  Node * next = new Node(Color);
  Node * curr = this;

  while(curr->next != NULL){
    curr = curr->next;
  }
  curr->next  = next; 
}


void Node::remove_last(){
  Node * curr = this;

  if(this->next == NULL){
    delete this->next;
    this->next = NULL;
    return;
  }

  while(curr->next->next != NULL){
    curr = curr->next;
  }
  delete curr->next;
  curr->next  = NULL; 
  return;
}
