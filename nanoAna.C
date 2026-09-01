#define nanoAna_cxx
#include <TH2.h>
#include <TStyle.h>

//Standard headers:
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

//Headers for this setup
#include "nanoAna.h" //Loads the analysis class, ONNX and the dynamic readers 
#include "headers/Init.h"
#include "headers/BookHistograms.h"
#include "headers/CustomFunctions.h"
#include "headers/LoadGoldenJSONs.h"
#include "headers/ProduceGenCollection.h"
#include "headers/ProduceRecoCollection.h"
#include "headers/VetoEvents.h"
#include "headers/AnalysisLogic.h"

void nanoAna::Begin(TTree * /*tree*/){TString option = GetOption();}
void nanoAna::SlaveBegin(TTree *tree)
{
  //------------------------------------------------------------------
  // Note on _era (TString):
  // Make sure to use the same naming convention for _era.
  // It is used to make year/campaign dependent choices downstream.
  // The following options are recommended.
  // 2016-preVFP, 2016-postVFP, 2017, 2018,
  // 2022-preEE, 2022-postEE, 2023-preBPix, 2023-postBPix
  // new: 2024, 2025
  //------------------------------------------------------------------
  
  TString option = GetOption();

  // Set year and b-tagging working point from _era:
  // b-tagging threshold source: https://btv-wiki.docs.cern.ch/ScaleFactors/
  _btaggingWP = -1; _year = -1;
  if      (_era.Contains("2016-pre"))  {_btaggingWP = 0.2598; _year = 2016;}
  else if (_era.Contains("2016-post")) {_btaggingWP = 0.2489; _year = 2016;}
  else if (_era.Contains("2017"))      {_btaggingWP = 0.3040; _year = 2017;}
  else if (_era.Contains("2018"))      {_btaggingWP = 0.2783; _year = 2018;}
  else if (_era.Contains("22-pre"))    {_btaggingWP = 0.3086; _year = 2022;}
  else if (_era.Contains("22-post"))   {_btaggingWP = 0.3196; _year = 2022;}
  else if (_era.Contains("23-pre"))    {_btaggingWP = 0.2431; _year = 2023;}
  else if (_era.Contains("23-post"))   {_btaggingWP = 0.2435; _year = 2023;}
  else if(_era.Contains("24"))         {_year = 2024;}
  if(_year < 0 ) cout<<"\033[31m[ERROR] Could not set year. Provide correct _era.\033[0m"<<endl;
  if(_btaggingWP < 0 ) cout<<"\033[31m[ERROR] Could not set b-tagging WP. Provide correct _era.\033[0m"<<endl;

  cout<<"\033[33m"<<endl;
  cout<<"Input parameters:"<<endl;
  cout<<"  - Data   : "<<_data<< " (0=MC, 1=Data)" <<endl;
  cout<<"  - Sample : "<<_sample<<endl;
  cout<<"  - Era    : "<<_era<<endl;
  cout<<"  - b-WP   : "<<_btaggingWP<<endl;
  cout<<"  - Year   : "<<_year<<endl;
  cout<<"\033[0m"<<endl;
  
  // Initialization of the counters:
  time(&start);
  nEvtTotal      = 0;
  nEvtRan        = 0;
  nEvtTrigger    = 0;
  nEvtPass       = 0;
  nThrown        = 0;
  genEventSumW   = 0;
  nevt_OS=0; nevt_OSmm=0; nevt_OSme=0; nevt_OSem=0; nevt_OSee=0;
  nevt_SS=0; nevt_SSmm=0; nevt_SSme=0; nevt_SSem=0; nevt_SSee=0;
  nevt_3L4L=0; nevt_HEM=0; nevt_EE=0;

  // Constants:
  nEvtGen = tree->GetEntries(); 
  test_event = 100;
  cout << ">> Total tree entries: " << nEvtGen << endl;

  // Load Golden JSON data:
  goldenJSONdata = loadGoldenJSON();
  
  // Create a TFile to write on.
  // Call any other function that does any kind of initialization of objects/variables.
  _HstFile = new TFile(_HstFileName,"recreate");
  BookHistograms();

  // ----------------------------------------------------------------------------------
  // Loading DNNs and scaling parameters:

  // One environment to rule them all
  ort_env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "MultiDNN_Inference");
  cout << ">> Initialized ONNX environment." << endl;
  session_options.SetIntraOpNumThreads(1);

  //Load all DNNs and scaling parameters here
  loadAllDNNs();
  
  //-----------------------------------------------------------------------------------

  // Header for progress on screen:
  cout << "\n"
     << right << setw(8) << "Progress"
     << right << setw(12) << "nEvents"
     << right << setw(8) << "Time" << endl;
}
void nanoAna::SlaveTerminate()
{
  float goodevtfrac = ((float)nEvtRan)/((float)nEvtTotal);
  float trigevtfrac = ((float)nEvtTrigger)/((float)nEvtTotal);
  float passevtfrac = ((float)nEvtPass)/((float)nEvtTotal);
  float notgoldenevtfrac  = ((float)nThrown)/((float)nEvtTotal);

  cout<<"---------------------------------------------"<<endl;
  cout<<"Summary:"<<fixed << setprecision(6)<<endl;
  cout<<"nEvtTotal   = "<<nEvtTotal<<endl;
  cout<<"nEvtRan     = "<<nEvtRan<<" ("<<goodevtfrac*100<<" %)"<<endl;
  cout<<"nEvtTrigger = "<<nEvtTrigger<<" ("<<trigevtfrac*100<<" %)"<<endl;
  cout<<"nEvtPass    = "<<nEvtPass<<" ("<<passevtfrac*100<<" %)"<<endl;
  if(_data==1) cout<<"nEvents not in golden json = "<<nThrown<<" ("<<notgoldenevtfrac*100<<" %)"<<endl;
  if(_data==0) cout<<"Sum of generator weights = "<<fixed<<setprecision(2)<<genEventSumW<<endl;
  cout<<"---------------------------------------------"<<endl;

  // Analysis specific event counters:
  cout<<"nEvt 2LOS   : "<<nevt_OS<<endl;
  cout<<"   -> mm    : "<<nevt_OSmm<<endl;
  cout<<"   -> me    : "<<nevt_OSme<<endl;
  cout<<"   -> em    : "<<nevt_OSem<<endl;
  cout<<"   -> ee    : "<<nevt_OSee<<endl;
  cout<<"---------------------------------------------"<<endl;
  
  cout<<"nEvt 2LSS   : "<<nevt_SS<<endl;
  cout<<"   -> mm    : "<<nevt_SSmm<<endl;
  cout<<"   -> me    : "<<nevt_SSme<<endl;
  cout<<"   -> em    : "<<nevt_SSem<<endl;
  cout<<"   -> ee    : "<<nevt_SSee<<endl;
  cout<<"---------------------------------------------"<<endl;
  cout<<"Events rejected due to 3L4L overlap: "<<nevt_3L4L<<endl;
  cout<<"Events rejected due to HEM issue   : "<<nevt_HEM<<" (2018)"<<endl;
  cout<<"Events rejected due to EE issue    : "<<nevt_EE<<" (2022-preEE)"<<endl;
  
  //----------------------------------------------------------------
  //Storing event information in a histogram:
  // Navigate to the file's directory in ROOT memory before creating histograms
  _HstFile->cd();
  TH1D *hCount = new TH1D("hCount", "hCount;;", 3, 0.5, 3.5);
  hCount->SetBinContent(1,nEvtTotal); //-> luminosity calculation
  hCount->SetBinContent(2,nEvtRan);
  hCount->SetBinContent(3,nEvtTrigger);
  hCount->GetXaxis()->SetBinLabel(1,"nEvtGen");
  hCount->GetXaxis()->SetBinLabel(2,"nEvtRan");
  hCount->GetXaxis()->SetBinLabel(3,"nEvtTrigger");

  TH1D *hWt = new TH1D("hWt", "hWt;;", 1, 0, 1);
  hWt->SetBinContent(1,genEventSumW);
  hWt->GetXaxis()->SetBinLabel(1,"genEventSumW"); //-> gen-level weights
  //----------------------------------------------------------------

  // Close the file AFTER writing everything
  _HstFile->Write();
  _HstFile->Close();

  // Print time taken:
  time(&end);
  int time_taken = int(end - start);
  int H = time_taken / 3600;
  int M = (time_taken % 3600) / 60;
  int S = time_taken % 60;
  cout << "\n>> Process time: \033[34m"<< Form("%02d:%02d:%02d", H, M, S)<< "\033[0m" << endl;
}
void nanoAna::Terminate(){}

//============================================================================
bool nanoAna::Process(Long64_t entry)
{
  // if _era/_data contains invalid values, use Abort() and display an error message in red.
  
  nEvtTotal++; //Counts the total number of events processed from the file
  
  // The following fReader can access both data and MC branches.
  // Any branch that is not in the ROOT file returns a default value.
  // (For example, gen-level branches in data.)
  // MUST load the entry first before accessing ANY branch values!
  fReader.SetLocalEntry(entry);

  // Count the sum of generator weights for later use.
  if(_data==0) genEventSumW += (float) *Generator_weight;

  // Setting verbosity:
  if (nEvtTotal % _verbosity == 0) {
    time(&buffer);
    double time_buff = double(buffer-start);
    double frac = (double)nEvtTotal / nEvtGen * 100.0;
    string progress = (ostringstream() << fixed << setprecision(2) << frac << "%").str();
    cout << right << setw(8) << progress
	 << right << setw(12) << nEvtTotal
	 << right << setw(8) << fixed << setprecision(0) << time_buff
	 << setprecision(6) << endl;
  }

  //-----------------------------------------------------------------------------------------------
  // MET filters:
  // 1. Flag_goodVertices = Primary vertex filter.
  // 2. Flag_globalSuperTightHalo2016Filter = Removes events affected by beam halo muons.
  // 3. Flag_EcalDeadCellTriggerPrimitiveFilter = Removes events where dead ECAL cells affect trigger primitives.   
  // 4. Flag_BadPFMuonFilter = Removes events containing misreconstructed PF muons.
  // 5. Flag_BadPFMuonDzFilter = Additional filter for PF muons with large dz.
  // 6. Flag_HBHENoiseFilter = Rejects events with noise from the HCAL.
  // 7. Flag_HBHENoiseIsoFilter = Removes isolated HCAL noise not caught by the standard filter.
  // 8. Flag_eeBadScFilter  = Removes events with problematic superclusters in ECAL endcap.
  // 9. Flag_ecalBadCalibFilter = Rejects events with problematic ECAL crystals due to bad calibrations.
  //
  // Things to keep in mind:
  // - Flag_HBHENoiseFilter and Flag_HBHENoiseIsoFilter can be replaced with Flag_hfNoisyHitsFilter.
  // - Do not use the default Flag_ecalBadCalibFilter for 2022 and 2023 (NanoAODv12).
  // - Flag_BadChargedCandidateFilter is not recommended.
  // Reference: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2
  //-----------------------------------------------------------------------------------------------

  GoodEvt =
    *Flag_goodVertices &&
    *Flag_globalSuperTightHalo2016Filter &&
    *Flag_EcalDeadCellTriggerPrimitiveFilter &&
    *Flag_BadPFMuonFilter &&
    *Flag_BadPFMuonDzFilter &&
    *Flag_hfNoisyHitsFilter && // both 6 and 7
    *Flag_eeBadScFilter &&
    (_year<2019 ? *Flag_ecalBadCalibFilter: 1);
  
  if(GoodEvt){
    nEvtRan++;  //Total number of good events
    bool trigger = true; //default, always true for MC
      
    if(_data==1){
      // Construct the trigger logic using the HLT_* branches and the _.
      // It may be different for sample with names "Muon"/"Electron/" or the year.
      // Note: Dafault not set: code will break if the HLT path is not found in the file (by design).
      bool trigger_mu  = true;  bool trigger_ele = true;
      if     (_year == 2016){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele27_WPTight_Gsf;}
      else if(_year == 2017){trigger_mu = *HLT_IsoMu27; trigger_ele=*HLT_Ele32_WPTight_Gsf_L1DoubleEG;}
      else if(_year == 2018){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele32_WPTight_Gsf;}
      else if(_year == 2022){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele32_WPTight_Gsf;}
      else if(_year == 2023){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele32_WPTight_Gsf;}
      else cout<<"\033[31m[WARNING] nanoAna.C: trigger path not chosen for ("<<_year<<")\033[0m"<<endl;
      bool overlapping_events = trigger_mu && trigger_ele;
      
      //Applying trigger on data: (logic: overlapping events are taken from the muon dataset)
      if(_sample.Contains("Muon"))   trigger = trigger_mu;                 //includes overlap.
      if(_sample.Contains("EGamma")) trigger = trigger_ele && !trigger_mu; //excludes overlap.
      if(_sample.Contains("SingleElectron")) trigger = trigger_ele && !trigger_mu; //excludes overlap.
	
      //Throw awaying bad data that are not included in the GoldenJSON:
      int runno = (int)*run;
      int lumisection = (int)*luminosityBlock;
      bool golden_event = checkGoldenJSON(runno, lumisection);
      if(!golden_event){
	nThrown++;
	trigger = false;
      }
    }

    if (trigger){
      nEvtTrigger++;
    
      //The analysis is done for the good events which pass the trigger.

      /////////////////////////////////////
      //  Construction of the arrays:
      /////////////////////////////////////

      //---------------------
      // Reco-level objects
      //---------------------

      metpt = *PuppiMET_pt; metphi = *PuppiMET_phi; //nominal
      //metpt = *PuppiMET_phiUnclusteredUp;   metphi = *PuppiMET_phiUnclusteredUp; //unclustered-up
      //metpt = *PuppiMET_phiUnclusteredDown; metphi = *PuppiMET_phiUnclusteredDown; //unclustered-down

      // Clear the object arrays to get rid of entries from the previous event
      Muon.clear();        LooseMuon.clear();
      Electron.clear();    LooseElectron.clear();
      LightLepton.clear(); LooseLepton.clear();
      Jet.clear();         MediumbJet.clear();
     
      // Create the arrays
      createLightLeptons();
      createJets();

      // Sort them in pT
      SortPt(Muon);        SortPt(LooseMuon);
      SortPt(Electron);    SortPt(LooseElectron);
      SortPt(LightLepton); SortPt(LooseLepton);
      SortPt(Jet);         SortPt(MediumbJet);

      /////////////////////////////////////
      //          Analysis block
      /////////////////////////////////////

      // Reset global event flags for this event //IMPORTANT
      evt_2L = false; evt_2LOS = false; evt_2LSS = false;
      evt_mm = false; evt_me = false; evt_em = false; evt_ee = false;
      
      // Pick 2L events:
      if((int)LightLepton.size() == 2){

	// 1. Putting offline cuts on the leptons:
	bool offline_trigger = false;
	float ptcut_ele = 35;      float ptcut_mu = 26; // default
	if(_year==2016) {ptcut_ele = 30; ptcut_mu = 26;}
	if(_year==2017) {ptcut_ele = 35; ptcut_mu = 29;}
	
	for(int i=0; i<(int)LightLepton.size(); i++){
	  int lepton_id = fabs(LightLepton.at(i).id);
	  float lepton_pt = LightLepton.at(i).v.Pt();
	  if(lepton_id == 11 && lepton_pt > ptcut_ele) offline_trigger = true;
	  if(lepton_id == 13 && lepton_pt > ptcut_mu)  offline_trigger = true;
	}

	// 2. Veoting events with additional loose leptons:
	if(LooseLepton.size() > 2) offline_trigger = false;

	// 3. Reject resonances:
	bool reject_low_resonances = (LightLepton.at(0).v + LightLepton.at(1).v).M() > 15; //Used for 2LSS
	bool reject_most_resonances = (LightLepton.at(0).v + LightLepton.at(1).v).M() > 150; // Used for 2LOS

	// 4. Define 2LOS/2LSS
	bool samesign = LightLepton.at(0).charge == LightLepton.at(1).charge;
	if(offline_trigger && reject_low_resonances && samesign)   {evt_2LSS = true;}
	if(offline_trigger && reject_most_resonances && !samesign) {evt_2LOS = true;}

	// 5. Veto additional events
	bool veto_3L4L_event = Veto3L4L();
	bool veto_HEM_event  = VetoHEM(Jet);
	bool veto_EE_event   = VetoEE(Jet);
	bool veto_this_event = veto_3L4L_event || veto_HEM_event || veto_EE_event;
	if(veto_this_event){
	  evt_2LSS = false;
	  evt_2LOS = false;
	}

	// Final selection of 2L events
	evt_2L = evt_2LOS || evt_2LSS;
	
	// Check flavor channel of the event:
	int flav0 = fabs(LightLepton.at(0).id);
	int flav1 = fabs(LightLepton.at(1).id);
	if(     flav0 == 13 && flav1 == 13) evt_mm = true;
	else if(flav0 == 13 && flav1 == 11) evt_me = true;
	else if(flav0 == 11 && flav1 == 13) evt_em = true;
	else if(flav0 == 11 && flav1 == 11) evt_ee = true;

	// Counting events:
	if(evt_2LOS){
	  nevt_OS++;
	  if(evt_mm) nevt_OSmm++;
	  if(evt_me) nevt_OSme++;
	  if(evt_em) nevt_OSem++;
	  if(evt_ee) nevt_OSee++;
	}
	if(evt_2LSS){
	  nevt_SS++;
	  if(evt_mm) nevt_SSmm++;
	  if(evt_me) nevt_SSme++;
	  if(evt_em) nevt_SSem++;
	  if(evt_ee) nevt_SSee++;
	}

	/////////////////////////////////////////////
	// Pick which event to run the analysis for:
	/////////////////////////////////////////////
	if(evt_2LOS && evt_mm){
	  nEvtPass++;
	  runAnalysis();
	}

      }// 2L events
    }// trigger
  }//GoodEvt
  return true;
}



