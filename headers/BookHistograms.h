void nanoAna::BookHistograms()
{
  //The histograms are booked here.
  //Binning etc are done here.
  //These histograms are stored in the hst_<process name>.root file in the same order.

  h.hist.resize(3);
  //Example : new TH1F ("hst_name", "hst title", total bins, initial x, final x);
  h.hist[0] = new TH1F("leading_muon_pT", "leading muon pT", 200, 0, 200);
  h.hist[1] = new TH1F("fixedGridRhoFastjetAll", "fixedGridRhoFastjetAll", 200, 0, 200);
  h.hist[2] = new TH1F("dnn_score_1", "dnn_score_1", 100, 0, 1);
  for(int i = 0; i < (int)h.hist.size(); i++) h.hist[i]->Sumw2();

  h.dnn.resize(6);
  h.dnn[0] = new TH1F("var0_dilep_dphi", "dilep_dphi", 100, 0, 6);
  h.dnn[1] = new TH1F("var1_dilep_eta",  "dilep_eta",  100, -10, 10);
  h.dnn[2] = new TH1F("var2_dilep_ptratio", "dilep_ptratio", 100, 0, 1);
  vector<float> GEVbins = {0, 50, 100, 200, 300, 400, 500};
  h.dnn[3] = new TH1F("var3_HT",  "HT",  GEVbins.size()-1, &GEVbins[0]);
  h.dnn[4] = new TH1F("var4_LT",  "LT",  GEVbins.size()-1, &GEVbins[0]);
  h.dnn[5] = new TH1F("var5_MET", "MET", GEVbins.size()-1, &GEVbins[0]);
  for(int i = 0; i < (int)h.dnn.size(); i++) h.dnn[i]->Sumw2();

  //Analysis plots:
  h.ana.resize(1);
  h.ana[0] = new TH1F("LTplusMET", "LTplusMET", 4, std::vector<double>{0,400,500,600,650}.data());
  for(int i = 0; i < (int)h.ana.size(); i++) h.ana[i]->Sumw2();

  //Weights:
  h.evtwt.resize(16);
  h.evtwt[0]  = new TH1F("wt_lumWeight", "lumWeight", 100, -5000, 5000);
  h.evtwt[1]  = new TH1F("wt_genWeight", "genWeight", 100, -5, 5);
  h.evtwt[2]  = new TH1F("wt_eleReco",   "eleReco",   100, 0, 2);
  h.evtwt[3]  = new TH1F("wt_eleIdIso",  "eleIdIso",  100, 0, 2);
  h.evtwt[4]  = new TH1F("wt_muReco",    "muReco",    100, 0, 2);
  h.evtwt[5]  = new TH1F("wt_muId",      "muId",      100, 0, 2);
  h.evtwt[6]  = new TH1F("wt_muIso",     "muIso",     100, 0, 2);
  h.evtwt[7]  = new TH1F("wt_trigEff",   "trigEff",   100, 0, 2);
  h.evtwt[8]  = new TH1F("wt_pileup",    "pileup",    100, 0, 2);
  h.evtwt[9]  = new TH1F("wt_pdf",       "pdf",       100, 0, 2);
  h.evtwt[10] = new TH1F("wt_qcdscale",  "qcdscale",  100, 0, 2);
  h.evtwt[11] = new TH1F("wt_prefiring", "prefiring", 100, 0, 2);
  h.evtwt[12] = new TH1F("wt_mc_custom", "mc_custom", 100, 0, 2);
  h.evtwt[13] = new TH1F("wt_POG",       "POG",       100, 0, 2);
  h.evtwt[14] = new TH1F("wt_theory",    "theory",    100, 0, 2);
  h.evtwt[15] = new TH1F("wt_evt",       "evtwt",     100, 0, 2);
  //for(int i = 0; i < (int)h.evtwt.size(); i++) h.evtwt[i]->Sumw2();
}

void nanoAna::FillHistograms(float wt){

  h.ana[0] -> Fill(LTplusMET, wt);
  
}
