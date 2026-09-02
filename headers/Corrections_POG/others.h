//------------------------------------
// PDF, LHE and L1prefire corrections
//------------------------------------

double nanoAna::returnPDFweight(string mode){
  double weight = 1.0;
  if(_data==0){
    const auto n = LHEPdfWeight.GetSize();
    if(n > 1){
      double pdf_up = LHEPdfWeight[1];
      double pdf_down = LHEPdfWeight[1];
      for(unsigned int i=1; i<n; ++i){
        pdf_up   = max(pdf_up,   (double)LHEPdfWeight[i]);
        pdf_down = min(pdf_down, (double)LHEPdfWeight[i]);
      }
      if(mode=="systup")        weight = pdf_up;
      else if(mode=="systdown") weight = pdf_down;
    }
  }
  return weight;
}

double nanoAna::returnQCDscaleWeight(string mode){
  double weight = 1.0;
  if(_data==0){
    const auto n = LHEScaleWeight.GetSize();
    if(n > 7){
      double qcd_up   = LHEScaleWeight[1];
      double qcd_down = LHEScaleWeight[1];
      int idx[4] = {1,3,5,7};
      for(int k=0; k<4; ++k){
        qcd_up   = max(qcd_up,   (double)LHEScaleWeight[idx[k]]);
        qcd_down = min(qcd_down, (double)LHEScaleWeight[idx[k]]);
      }
      if(mode=="systup")        weight = qcd_up;
      else if(mode=="systdown") weight = qcd_down;
    }
  }
  return weight;
}

double nanoAna::returnL1prefireWeight(TString mode){
  double weight = 1.0;
  if(_year <=2017) {
    if(mode == "nom")           weight = *L1PreFiringWeight_Nom;
    else if (mode=="systup")    weight = *L1PreFiringWeight_Up;
    else if (mode=="systdown")  weight = *L1PreFiringWeight_Dn;
  }
  return weight;
}
