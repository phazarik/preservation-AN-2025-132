void nanoAna::RunAnalysis(){
  
  // IMPORTANT: This function should be called for a subset of 2L events

  // Pick MET: (use Up/Down for systematic variations)
  metpt = *PuppiMET_pt; metphi = *PuppiMET_phi; //nominal

  // Tweak Jet 4-vectors for systematic variations

  // Tweak lepton 4-vectors for systematic variations

  AssignVarValues(); // Call this after MET and 4-vectors are fixed.

  ///////////////////////////////////
  //    CALCULATE EVENT WEIGHTS
  ///////////////////////////////////

  float lumWeight = 1.0;
  float genWeight  = 1.0;
  float evtWeight = 1.0;

  if(_data==0){

    // -----------------------------------------------------------------------------------
    // 1. Luminosity weight:
    // Extract lumWeight from external JSON file, or keep it as 1, and fix during plotting.
    // Luminosity = sample cross-section/nEvtTotal.
    // The denominator keeps chaning based on how many events were processed on CRAB/REANA. 
    // Recommended: Calculate it beforehand for each sample and use it here using a {_sample: value} JSON.

    // -----------------------------------------------------------------------------------
    // 2. Generator-level weight:
    // Use the sum of generator weight for the complete MC sample (genEventSumW from SlaveTerminate)
    // Externally, calculate avgGenWeight = genEventSumW/nEvtGen for the complete sample.
    // Read it from external JSON file as {_sample: value}.
    // genWeight = *Generator_weight/avgGenWeight;

    // -----------------------------------------------------------------------------------
    // For the purpose of preserving this analysis in REANA,
    // lumWeight and genWeight are not implemented here as they depend on
    // the output of SlaveTerminate() {nEvtTotal and genEventSumW} for the entire sample.
    // The external JSON file keeping these numbers will change from user to user.

    // 3. Corrections from POGs:
    auto safeSF = [](double sf){ return (sf<0) ? 1.0 : sf; }; //avoid negatives, just in case
    double wt_eleReco  = 1.0;
    double wt_eleIdIso = 1.0;
    double wt_muReco   = 1.0;
    double wt_muId     = 1.0;
    double wt_muIso    = 1.0;
    
    for(int i = 0; i < 2; i++){
      const auto& lep = LightLepton.at(i);
      int id = abs(lep.id);
      if(id == 11){
        wt_eleReco  *= safeSF(correctionlib_electron_RecoSF(lep.v, "nom"));
        wt_eleIdIso *= safeSF(correctionlib_electron_IDSF(lep.v, "nom")); // Fixed capitalization
      }
      else if(id == 13){
        wt_muReco *= safeSF(correctionlib_muon_RecoSF(lep.v, "nom"));
        wt_muId   *= safeSF(correctionlib_muon_IDSF(lep.v, "nom"));
        wt_muIso  *= safeSF(correctionlib_muon_IsoSF(lep.v, "nom"));
      }
    }
    
    double ef0 = GetLeptonTriggerEfficiency(LightLepton.at(0), "nom");
    double ef1 = GetLeptonTriggerEfficiency(LightLepton.at(1), "nom");
    double wt_trigEff   = 1.0 - (1.0 - ef0) * (1.0 - ef1);    
    double wt_pileup    = safeSF(correctionlib_pileupSF("nom"));
    double wt_pdf       = returnPDFweight("up");
    double wt_qcdscale  = returnQCDscaleWeight("up");
    double wt_prefiring = returnL1prefireWeight("nom");

    // 4. Analysis specific corrections:
    double wt_mc_custom = 1.0;
    const vector<TString> dySamples = {"DYto2L", "DYtoLL", "DYGToLLG"};
    const vector<TString> ttSamples = {"TT_", "TTV"};
    const vector<TString> qcdSamples = {"QCDMu", "QCDEM"};
    const vector<TString> wjSamples = {"WtoLNu", "WGtoLNuG"};
    auto containsAny = [&](const vector<TString>& samples) {
      for (const auto& sample : samples)
	if (_sample.Contains(sample)) return true;
      return false;
    };
    if (containsAny(dySamples)) wt_mc_custom *= returnBinnedSF(sf_dy, LTplusMET, "nom");
    else if (containsAny(ttSamples)){
      wt_mc_custom *= returnGlobalSF(sf_ttx_global, "nom");
      wt_mc_custom *= returnBinnedSF(sf_ttx, HT, "nom");
    }
    else if (containsAny(qcdSamples)) wt_mc_custom *= returnGlobalSF(sf_qcd, "nom");
    else if (containsAny(wjSamples))  wt_mc_custom *= returnGlobalSF(sf_wj, "nom");
    
    /* TEST
       printf("Test weights: lum=%.6f gen=%.6f eleReco=%.6f eleIdIso=%.6f muReco=%.6f muId=%.6f muIso=%.6f trigEff=%.6f pileup=%.6f pdf=%.6f qcdscale=%.6f prefiring=%.6f mc_custom=%.6f\n",
	   lumWeight, genWeight, wt_eleReco, wt_eleIdIso, wt_muReco, wt_muId,
	   wt_muIso, wt_trigEff, wt_pileup, wt_pdf, wt_qcdscale, wt_prefiring,
	   wt_mc_custom);*/

    // Combine weights:
    double wt_POG = wt_eleReco * wt_eleIdIso * wt_muReco * wt_muId * wt_muIso * wt_pileup;
    double wt_theory = wt_pdf * wt_qcdscale;
    evtWeight = lumWeight * genWeight * wt_POG * wt_theory * wt_trigEff * wt_prefiring * wt_mc_custom;
    
    h.evtwt[0]->Fill(lumWeight);
    h.evtwt[1]->Fill(genWeight);
    h.evtwt[2]->Fill(wt_eleReco);
    h.evtwt[3]->Fill(wt_eleIdIso);
    h.evtwt[4]->Fill(wt_muReco);
    h.evtwt[5]->Fill(wt_muId);
    h.evtwt[6]->Fill(wt_muIso);
    h.evtwt[7]->Fill(wt_trigEff);
    h.evtwt[8]->Fill(wt_pileup);
    h.evtwt[9]->Fill(wt_pdf);
    h.evtwt[10]->Fill(wt_qcdscale);
    h.evtwt[11]->Fill(wt_prefiring);
    h.evtwt[12]->Fill(wt_mc_custom);
    h.evtwt[13]->Fill(wt_POG);
    h.evtwt[14]->Fill(wt_theory);
    h.evtwt[15]->Fill(evtWeight);
  }

  ///////////////////////////////////
  //     CORE OF THE ANALYSIS
  ///////////////////////////////////

  bool CR_ttx   = false; bool VR_2LOS = false;
  bool CR_dy    = false; bool VR_2LSS = false;
  bool CR_qcd   = false; bool SR_2LOS = false;
  bool CR_wjets = false; bool SR_2LSS = false;
  bool killqcd  = dilep_deta<3 && lep0_iso<0.05 && lep0_sip3d<5;
  
  // Regions in 2LOS based on DNN1-DNN2 plane
  if(evt_2LOS){
    CR_ttx  = killqcd && dnn1<0.50 && 0.50<dnn2;
    CR_dy   = killqcd && njet<2    && dnn2<0.15;
    VR_2LOS = killqcd && (( dnn1<0.50 && 0.15<dnn2 && dnn2<0.50 )||
			  ( 0.50<dnn1 && 0.15<dnn2 && dnn2<0.65 )); 
    SR_2LOS = killqcd && 0.60<dnn1 && 0.70<dnn2;
  }

  // Regions in 2LSS based on DNN3-DNN4 plane
  if(evt_2LSS){
    CR_qcd   = dnn3<0.20 && lep0_iso>0.03;
    CR_wjets = killqcd && 0.2<dnn3 && dnn4<0.4; 
    VR_2LSS  = killqcd && (( 0.2<dnn3 && dnn3<0.6 && 0.4<dnn4 )||
			   ( 0.6<dnn3 && 0.4<dnn4 && dnn4<0.7 ));
    SR_2LSS  = killqcd && dnn3<0.6 && dnn4<0.7;
  }

  /////////////////////////////////////////
  // SELECT EVENTS + LEPTON FLAVOR CHANNEL
  /////////////////////////////////////////
  
  if(SR_2LOS && evt_mm){
    nEvtPass ++;
    FillHistograms(evtWeight);
  }
	
}

void nanoAna::AssignVarValues(){
  // Assign values to the globally declared variables
  // that are relevant to this analysis.

  // Event level variables
  nlep  = (int)LightLepton.size();
  njet  = (int)Jet.size();
  nbjet = (int)MediumbJet.size();

  HT = 0; for(int i=0; i<(int)Jet.size(); i++) HT += Jet.at(i).v.Pt();
  LT = LightLepton.at(0).v.Pt() + LightLepton.at(1).v.Pt();
  LTplusMET = LT+metpt; // -->Discriminating variable
  STvis = HT+LT;
  ST = HT+LT+metpt;
  STfrac = (ST!=0) ? LT/ST : 0.0;
	  
  // The dilepton system
  TLorentzVector dilep = LightLepton.at(0).v + LightLepton.at(1).v;
  dilep_pt   = dilep.Pt();
  dilep_eta  = dilep.Eta();
  dilep_phi  = dilep.Phi();
  dilep_mass = dilep.M();
  dilep_mt   = TransvMass(dilep);
	
  // Difference between the two leptons
  dilep_dR = LightLepton.at(0).v.DeltaR(LightLepton.at(1).v);
  dilep_dphi = DeltaPhi(LightLepton.at(0).v.Phi(), LightLepton.at(1).v.Phi());
  dilep_deta = fabs(LightLepton.at(0).v.Eta() - LightLepton.at(1).v.Eta());
  dilep_ptratio = LightLepton.at(1).v.Pt()/LightLepton.at(0).v.Pt();

  // Leading lepton
  lep0_pt  = LightLepton.at(0).v.Pt();
  lep0_eta = LightLepton.at(0).v.Eta();
  lep0_phi = LightLepton.at(0).v.Phi();
  lep0_mt  = TransvMass(LightLepton.at(0).v);
  if(abs(LightLepton.at(0).id)==11) lep0_iso = LightLepton.at(0).reliso03;
  else                              lep0_iso = LightLepton.at(0).reliso04;
  lep0_sip3d = (Float_t)LightLepton.at(0).sip3d;
	
  // Sub-leading lepton
  lep1_pt  = LightLepton.at(1).v.Pt();
  lep1_eta = LightLepton.at(1).v.Eta();
  lep1_phi = LightLepton.at(1).v.Phi();
  lep1_mt  = TransvMass(LightLepton.at(1).v);
  if(abs(LightLepton.at(1).id)==11) lep1_iso = LightLepton.at(1).reliso03;
  else                              lep1_iso = LightLepton.at(1).reliso04;
  lep1_sip3d = (Float_t)LightLepton.at(1).sip3d;

  // --------------------
  // Evaluate DNN scores
  // --------------------
	
  // DNN1: TTX-vs-VLLD
  vector<float> invar_ttx = {dilep_dphi, dilep_eta, dilep_ptratio, HT, LT, metpt}; //Maintain order
  dnn1 = evaluateDNN(session_ttx, invar_ttx, scale_min_ttx, scale_max_ttx, "input", "keras_tensor_3");
	
  // DNN2: DY-vs-VLLD
  vector<float> invar_dy = {dilep_dphi, dilep_eta, dilep_ptratio, HT, LT, metpt}; //Maintain order
  dnn2 = evaluateDNN(session_dy, invar_dy, scale_min_dy, scale_max_dy, "input", "keras_tensor_3");

  // DNN3: QCD-vs-VLLD
  vector<float> invar_qcd = {dilep_dphi, dilep_mass, dilep_ptratio, HT, LT, metpt}; //Maintain order
  dnn3 = evaluateDNN(session_qcd, invar_qcd, scale_min_qcd, scale_max_qcd, "input", "keras_tensor_3");

  // DNN4: Wjets-vs-VLLD
  vector<float> invar_wj = {dilep_deta, dilep_eta, dilep_ptratio, HT, LT, metpt}; //Maintain order
  dnn4 = evaluateDNN(session_wj, invar_wj, scale_min_wj, scale_max_wj, "input", "keras_tensor_3");
}
