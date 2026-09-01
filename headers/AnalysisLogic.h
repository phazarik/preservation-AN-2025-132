void nanoAna::runAnalysis()
{
  // This function should be called for a subset of 2L events

  // ---------------------------------------------------------
  // Assign values to the variables relevant to this analysis.
  // ---------------------------------------------------------

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
