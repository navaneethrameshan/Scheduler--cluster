//#include<iostream>
/*
#include<stdio.h>
#include<cstdlib>
#include"modules/tg.h"
#include"modules/file.h"
using namespace std;
*/
#include "modules/simulator.h"
#include "modules/logger.h"

int main(){

  Logger* l = Logger::getLogger();
  l->debug("Cloud simulator starts");
  
  Simulator *sim = new Simulator();
  sim->execute();

  l->finalize();
  return 0;
}
