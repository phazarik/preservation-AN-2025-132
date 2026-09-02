#include <utility>
#include <string>
#include <cmath>

pair<float,float> nanoAna::METXYCorr_Met_MetPhi(float metpt, float metphi){

  // tWiki: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETRun2Corrections
  // downloadable sources:
  // 1. https://mrieger.web.cern.ch/snippets/met_phi_correction.py (simplified for Run-2UL)
  // 2. https://lathomas.web.cern.ch/METStuff/XYCorrections/XYMETCorrection_withUL17andUL18andUL16.h

  int runnb = (int)*run;
  int npv = (int)*PV_npvs;
  
  if(npv > 100) npv = 100;
  string era = "";

  //Pick the MC eras (not subdivided)
  if(_data==0){
    if     (_campaign=="2016preVFP_UL")  era="yUL2016MCAPV";
    else if(_campaign=="2016postVFP_UL") era="yUL2016MCnonAPV";
    else if(_campaign=="2017_UL")        era="yUL2017MC";
    else if(_campaign=="2018_UL")        era="yUL2018MC";
    else return make_pair(1.0, 1.0);
  }

  // Pick data eras (subdivided into different run-no sections)
  else {
    if(runnb>=315252 && runnb<=316995)       era="yUL2018A";
    else if(runnb>=316998  && runnb<=319312) era="yUL2018B";
    else if(runnb>=319313  && runnb<=320393) era="yUL2018C";
    else if(runnb>=320394  && runnb<=325273) era="yUL2018D";
    else if(runnb>=297020  && runnb<=299329) era="yUL2017B";
    else if(runnb>=299337  && runnb<=302029) era="yUL2017C";
    else if(runnb>=302030  && runnb<=303434) era="yUL2017D";
    else if(runnb>=303435  && runnb<=304826) era="yUL2017E";
    else if(runnb>=304911  && runnb<=306462) era="yUL2017F";
    else if(runnb>=272007  && runnb<=275376) era="yUL2016B";
    else if(runnb>=275657  && runnb<=276283) era="yUL2016C";
    else if(runnb>=276315  && runnb<=276811) era="yUL2016D";
    else if(runnb>=276831  && runnb<=277420) era="yUL2016E";
    else if((runnb>=277772 && runnb<=278768) || runnb==278770) era="yUL2016F";
    else if((runnb>=278801 && runnb<=278808) || runnb==278769) era="yUL2016Flate";
    else if(runnb>=278820  && runnb<=280385) era="yUL2016G";
    else if(runnb>=280919  && runnb<=284044) era="yUL2016H";
    else return make_pair(1.0, 1.0);
  }

  //XY-corrections from POG (only available for Run-2)
  double cx=0.0, cy=0.0;

  //2016postVFP_UL
  if     (era=="yUL2016B")    { cx=-(-0.0214894*npv - 0.188255); cy=-(0.0876624*npv +0.812885); }
  else if(era=="yUL2016C")    { cx=-(-0.0322090*npv + 0.067288); cy=-(0.1139170*npv +0.743906); }
  else if(era=="yUL2016D")    { cx=-(-0.0293663*npv + 0.211060); cy=-(0.1133100*npv +0.815787); }
  else if(era=="yUL2016E")    { cx=-(-0.0132046*npv + 0.200730); cy=-(0.1348090*npv +0.679068); }
  else if(era=="yUL2016F")    { cx=-(-0.0543566*npv + 0.816597); cy=-(0.1142250*npv +1.172660); }
  else if(era=="yUL2016Flate"){ cx=-( 0.1346160*npv - 0.899650); cy=-(0.0397736*npv +1.038500); }
  else if(era=="yUL2016G")    { cx=-( 0.1218090*npv - 0.584893); cy=-(0.0558974*npv +0.891234); }
  else if(era=="yUL2016H")    { cx=-( 0.0868828*npv - 0.703489); cy=-(0.0888774*npv +0.902632); }

  //2016preVFP_UL
  else if(era=="yUL2016MCAPV")   { cx=-(-0.188743*npv + 0.136539); cy=-(0.01279270*npv +0.117747); }
  else if(era=="yUL2016MCnonAPV"){ cx=-(-0.153497*npv - 0.231751); cy=-(0.00731978*npv +0.243323); }

  //2017_UL
  else if(era=="yUL2017B") { cx=-(-0.211161*npv + 0.419333); cy=-(0.251789*npv - 1.280890); }
  else if(era=="yUL2017C") { cx=-(-0.185184*npv - 0.164009); cy=-(0.200941*npv - 0.568530); }
  else if(era=="yUL2017D") { cx=-(-0.201606*npv + 0.426502); cy=-(0.188208*npv - 0.583130); }
  else if(era=="yUL2017E") { cx=-(-0.162472*npv + 0.176329); cy=-(0.138076*npv - 0.250239); }
  else if(era=="yUL2017F") { cx=-(-0.210639*npv + 0.729340); cy=-(0.198626*npv + 1.028000); }
  else if(era=="yUL2017MC"){ cx=-(-0.300155*npv + 1.906080); cy=-(0.300213*npv - 2.022320); }

  //2018_UL
  else if(era=="yUL2018A") { cx=-(0.263733*npv - 1.911150); cy=-(0.0431304*npv - 0.112043); }
  else if(era=="yUL2018B") { cx=-(0.400466*npv - 3.059140); cy=-(0.1461250*npv - 0.533233); }
  else if(era=="yUL2018C") { cx=-(0.430911*npv - 1.428650); cy=-(0.0620083*npv - 1.460210); }
  else if(era=="yUL2018D") { cx=-(0.457327*npv - 1.568560); cy=-(0.0684071*npv - 0.928372); }
  else if(era=="yUL2018MC"){ cx=-(0.183518*npv + 0.546754); cy=-(0.1922630*npv - 0.421210); }

  float mx = metpt*cos(metphi) + cx;
  float my = metpt*sin(metphi) + cy;

  return make_pair(sqrt(mx*mx + my*my), atan2(my, mx));
}
