//----------------
// Light Leptons :
//----------------

void nanoAna::createLightLeptons(){

  //Muon block:
  unsigned int iterator_mu = (unsigned int)*nMuon;
  for(unsigned int i=0; i<iterator_mu ; i++){;
    Particle temp;
    temp.v.SetPtEtaPhiM(Muon_pt[i],Muon_eta[i],Muon_phi[i],0.105);
    temp.id = -13*Muon_charge[i];
    temp.ind = i;
    temp.charge = Muon_charge[i];
    temp.sip3d  = Muon_sip3d[i];
    temp.reliso03 = Muon_pfRelIso03_all[i];
    temp.reliso04 = Muon_pfRelIso04_all[i];
    temp.genindex = -1;
    temp.hovere   = -999;
    temp.r9       = -999;
    
    bool ptetacut = temp.v.Pt()>10 && fabs(temp.v.Eta())<2.4;
    bool promptmuon = fabs(Muon_dxy[i])<0.05 && fabs(Muon_dz[i])<0.1;
    bool passcut_loosemuon  = ptetacut && promptmuon && Muon_looseId[i] &&  Muon_pfRelIso04_all[i]<1.00;
    bool passcut_mediummuon = ptetacut && promptmuon && Muon_mediumId[i] && Muon_pfRelIso04_all[i]<0.15;
    passcut_mediummuon = passcut_mediummuon && Muon_tightCharge[i]>1;

    //More selections on sip3d
    bool passcuts_more = true;
    if     (_year >= 2018) passcuts_more = passcuts_more && Muon_sip3d[i] <  9.0;
    else if(_year == 2017) passcuts_more = passcuts_more && Muon_sip3d[i] < 12.0;
    else if(_year == 2016) passcuts_more = passcuts_more && Muon_sip3d[i] < 12.0;
    //More selections on jetIdx
    int jidx_mu = Muon_jetIdx[i];
    if(jidx_mu >= 0 && jidx_mu < (int)(*nJet)){
      if     (_year >= 2018) passcuts_more = passcuts_more && Jet_btagDeepFlavB[jidx_mu] < 0.3;
      else if(_year == 2017) passcuts_more = passcuts_more && Jet_btagDeepFlavB[jidx_mu] < 0.4;
      else if(_year == 2016) passcuts_more = passcuts_more && Jet_btagDeepFlavB[jidx_mu] < 0.6;
    }
    passcut_mediummuon = passcut_mediummuon && passcuts_more;

    // -------------------------------------------
    // For training DNNs with the QCD samples,
    //   - Set isolation selection to to 1.0
    //   - Decativate the tightCharge condition
    //   - Do not use passcuts_more
    // -------------------------------------------
    
    if(passcut_mediummuon){
      Muon.push_back(temp);
      LightLepton.push_back(temp);
    }
    if(passcut_loosemuon){
      LooseLepton.push_back(temp);
      LooseMuon.push_back(temp);
    }
  }//for muons

  //Electron block:
  unsigned int iterator_ele = (unsigned int)*nElectron;
  for(unsigned int i=0; i<iterator_ele; i++){
    Particle temp;
    temp.v.SetPtEtaPhiM(Electron_pt[i],Electron_eta[i],Electron_phi[i],0.000511); 
    temp.id = -11*Electron_charge[i];
    temp.ind = i;
    temp.charge   = Electron_charge[i];
    temp.sip3d    = Electron_sip3d[i];
    temp.reliso03 = Electron_pfRelIso03_all[i];
    temp.genindex = -1;
    temp.hovere   = Electron_hoe[i];
    temp.r9       = Electron_r9[i];

    bool ptetacut = temp.v.Pt()>10 && fabs(temp.v.Eta())<2.4;
    bool cleaned_from_muons = cleanFromArray(temp, LooseMuon, 0.05);
    bool isprompt = false;
    if(fabs(temp.v.Eta())<=1.479){//for barrel
      if(fabs(Electron_dxy[i])<0.05 && fabs(Electron_dz[i])<0.1)
	isprompt = true;
    }
    if(fabs(temp.v.Eta())>1.479){//endcap
      if(fabs(Electron_dxy[i])<0.1 && fabs(Electron_dz[i])<0.2)
	isprompt = true;
    }
    bool passcut_looseele  = (ptetacut && isprompt && (int)Electron_cutBased[i]>1
			      && Electron_pfRelIso03_all[i] < 1.0);
    bool passcut_mediumele = (ptetacut && isprompt && (int)Electron_cutBased[i]>2 &&
			      cleaned_from_muons && Electron_pfRelIso03_all[i] < 0.15);
    passcut_mediumele = passcut_mediumele && Electron_tightCharge[i]>1;

    //More selections on sip3d
    bool passcuts_more = true;
    if     (_year >= 2018) passcuts_more = passcuts_more && Electron_sip3d[i] <  9.0;
    else if(_year == 2017) passcuts_more = passcuts_more && Electron_sip3d[i] < 12.0;
    else if(_year == 2016) passcuts_more = passcuts_more && Electron_sip3d[i] < 12.0;
    //More selections on jetIdx
    int jidx_ele = Electron_jetIdx[i];
    if(jidx_ele >= 0 && jidx_ele < (int)(*nJet)){
      if     (_year >= 2018) passcuts_more = passcuts_more && Jet_btagDeepFlavB[jidx_ele] < 0.3;
      else if(_year == 2017) passcuts_more = passcuts_more && Jet_btagDeepFlavB[jidx_ele] < 0.4;
      else if(_year == 2016) passcuts_more = passcuts_more && Jet_btagDeepFlavB[jidx_ele] < 0.6;
    }
    passcut_mediumele = passcut_mediumele && passcuts_more;

    // -------------------------------------------
    // For training DNNs with the QCD samples,
    //   - Set isolation selection to to 1.0
    //   - Decativate the tightCharge condition
    //   - Do not use passcuts_more
    // -------------------------------------------

    if(passcut_mediumele){
      Electron.push_back(temp);
      LightLepton.push_back(temp);
    }
    if(passcut_looseele){
      LooseLepton.push_back(temp);
      LooseElectron.push_back(temp);
    }
  }//For electrons  
} 
//_____________________________________________________________________

void nanoAna::createJets(){
  unsigned int iterator_jet = (unsigned int)*nJet;
  for(unsigned int i=0; i<iterator_jet; i++){
    Particle temp;
    temp.v.SetPtEtaPhiM(Jet_pt[i],Jet_eta[i],Jet_phi[i],Jet_mass[i]);
    temp.ind = i;
    temp.btagscore = Jet_btagDeepFlavB[i];
    temp.hadronflavor = -1;
    if(_data==0) temp.hadronflavor = (int)Jet_hadronFlavour[i];

    bool ptetacut = temp.v.Pt()>50 && fabs(temp.v.Eta())<2.4;
    bool cleaned_from_leptons = cleanFromArray(temp, LooseLepton, 0.4);
    bool jetID = true; //NanoAODv15+ (2024 onwards) need a new jetID
    if (_year==2016)      jetID = (int)Jet_jetId[i]>=1;
    else if (_year<=2023) jetID = (int)Jet_jetId[i]>=2;
    bool passcut = ptetacut && cleaned_from_leptons && jetID;
    bool medium_bjet_pass = temp.btagscore > _btaggingWP;
    if(passcut && cleaned_from_leptons){
      Jet.push_back(temp);
      if(medium_bjet_pass) MediumbJet.push_back(temp);
    } 
  }
}

/*
void nanoAna::createFatJets() {
  unsigned int iterator_fatjet = (unsigned int)*nFatJet;
  for (unsigned int i = 0; i < iterator_fatjet; i++) {
    Particle temp;
    temp.v.SetPtEtaPhiM(FatJet_pt[i], FatJet_eta[i], FatJet_phi[i], FatJet_mass[i]);
    temp.ind = i;
    temp.btagscore = -1;           
    temp.hadronflavor = -1;
    if (_data == 0) temp.hadronflavor = (int)FatJet_hadronFlavour[i];
    bool ptetacut = temp.v.Pt() > 50 && fabs(temp.v.Eta()) < 2.4;
    bool cleaned_from_leptons = cleanFromArray(temp, LooseLepton, 0.8); //(since FatJets are large-cone)
    bool fatjetID = _year == 2016 ? (int)FatJet_jetId[i] >= 1 : (int)FatJet_jetId[i] >= 2;
    bool passcut = ptetacut && cleaned_from_leptons && fatjetID;
    if (passcut) FatJet.push_back(temp);
  }
  }*/

