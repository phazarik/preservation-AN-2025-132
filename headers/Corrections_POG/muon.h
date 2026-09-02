#include "correction.h"
#include <memory>
#include <string>
#include <TLorentzVector.h>
#include <filesystem>
using namespace std;

/*
  Input files:
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2016preVFP-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2016postVFP-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2017-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run2-2018-UL-NanoAODv9/2024-07-02/muon_Z.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-22CDSep23-Summer22-NanoAODv12/2025-08-14/muon_Z.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-22EFGSep23-Summer22EE-NanoAODv12/2025-08-14/muon_Z.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-23CSep23-Summer23-NanoAODv12/2025-08-14/muon_Z.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/MUO/Run3-23DSep23-Summer23BPix-NanoAODv12/2025-08-14/muon_Z.json.gz

  Keys used in Run2/Run3:
  - Run2: NUM_TrackerMuons_DEN_genTracks (reco), NUM_MediumID_DEN_genTracks (ID), NUM_TightRelIso_DEN_MediumID (Iso), None (HLT)
  - Run3: None (reco), NUM_MediumID_DEN_TrackerMuons (ID), NUM_TightPFIso_DEN_MediumID (Iso), NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight (HLT) 

  Input parameters:
  - float: fabs(eta) [0.0, 2.4)
  - float: pt [40.0, inf) for reco, [15.0, inf) for ID and Iso, [26, Inf) for HLT
  - string: mode [nominal, systdown, systup]
*/

static unique_ptr<correction::CorrectionSet> muon_json;
static const correction::Correction* muonReco;
static const correction::Correction* muonID;
static const correction::Correction* muonISO;
static const correction::Correction* muonHLT;

//------------------------------------------------------------
// Load MUO JSON
// Must be initialized before Process()
//------------------------------------------------------------
void nanoAna::initMuonCorrections() {

  const vector<int> validYears = {2016, 2017, 2018, 2022, 2023};
  if (find(validYears.begin(), validYears.end(), _year) == validYears.end()) {
    cout << ">> MUO corrections not available for year " << _year
	 << ", using SF = 1" << endl;
    muonReco = muonID = muonISO = muonHLT = nullptr;
    muon_json.reset();
    return;
  }

  string baseDir = filesystem::path(__FILE__).parent_path().string();
  string filePath = baseDir + "/JSONs/MUO/muon_Z_" + string(_campaign) + ".json.gz";
  if (FILE *f = fopen(filePath.c_str(), "r")) fclose(f);
  else { cout << "[ERROR] File not found\n"; return; }

  try {muon_json = correction::CorrectionSet::from_file(filePath);}
  catch (const exception &e) {cout << "Failed to load muon JSON: " << e.what() << endl; return;}
  
  if (_year < 2019) {
    muonReco = muon_json->at("NUM_TrackerMuons_DEN_genTracks").get();
    muonID   = muon_json->at("NUM_MediumID_DEN_genTracks").get();
    muonISO  = muon_json->at("NUM_TightRelIso_DEN_MediumID").get();
    muonHLT  = nullptr;
  }
  else {
    muonReco = nullptr;
    muonID   = muon_json->at("NUM_MediumID_DEN_TrackerMuons").get();
    muonISO  = muon_json->at("NUM_TightPFIso_DEN_MediumID").get();
    muonHLT  = muon_json->at("NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight").get();
  }    
  cout << ">> Loaded corrections: MUO" << endl;
}

//------------------
// Muon Reco SF
//------------------
double nanoAna::correctionlib_muon_RecoSF(const TLorentzVector &mu, const string &_mode) {
  if (!muonReco) return 1.0;

  float pt  = mu.Pt();
  float eta = fabs(mu.Eta());
  if (pt < 40. || eta > 2.4) return 1.0;

  string mode;
  if      (_mode=="nom")      mode="nominal";
  else if (_mode=="systup")   mode="systup";
  else if (_mode=="systdown") mode="systdown";
  else { cout<<"MUO: Wrong syst mode "<<_mode<<endl; return 1.0; }

  vector<correction::Variable::Type> values;
  values.emplace_back(eta);
  values.emplace_back(pt);
  values.emplace_back(mode);

  return muonReco->evaluate(values);
}

//----------------
// Muon ID SF
//----------------
double nanoAna::correctionlib_muon_IDSF(const TLorentzVector &mu, const string &_mode) {
  if (!muonID) return 1.0;

  float pt  = mu.Pt();
  float eta = fabs(mu.Eta());
  if (pt < 15. || eta > 2.4) return 1.0;

  string mode;
  if      (_mode=="nom")      mode="nominal";
  else if (_mode=="systup")   mode="systup";
  else if (_mode=="systdown") mode="systdown";
  else { cout<<"MUO: Wrong syst mode "<<_mode<<endl; return 1.0; }

  vector<correction::Variable::Type> values;
  values.emplace_back(eta);
  values.emplace_back(pt);
  values.emplace_back(mode);

  return muonID->evaluate(values);
}

//----------------
// Muon ISO SF
//----------------
double nanoAna::correctionlib_muon_IsoSF(const TLorentzVector &mu, const string &_mode) {
  if (!muonISO) return 1.0;

  float pt  = mu.Pt();
  float eta = fabs(mu.Eta());
  if (pt < 15. || eta > 2.4) return 1.0;

  string mode;
  if      (_mode=="nom")      mode="nominal";
  else if (_mode=="systup")   mode="systup";
  else if (_mode=="systdown") mode="systdown";
  else { cout<<"MUO: Wrong syst mode "<<_mode<<endl; return 1.0; }

  vector<correction::Variable::Type> values;
  values.emplace_back(eta);
  values.emplace_back(pt);
  values.emplace_back(mode);

  return muonISO->evaluate(values);
}

//----------------
// Muon HLT SF
//----------------
double nanoAna::correctionlib_muon_HLTSF(const TLorentzVector &mu, const string &_mode) {
  if (!muonHLT) return 1.0;

  float pt  = mu.Pt();
  float eta = fabs(mu.Eta());
  if (pt < 26. || eta > 2.4) return 1.0;

  string mode;
  if      (_mode=="nom")      mode="nominal";
  else if (_mode=="systup")   mode="systup";
  else if (_mode=="systdown") mode="systdown";
  else { cout<<"MUO: Wrong syst mode "<<_mode<<endl; return 1.0; }

  vector<correction::Variable::Type> values;
  values.emplace_back(eta);
  values.emplace_back(pt);
  values.emplace_back(mode);

  return muonHLT->evaluate(values);
}


//---------------------------------------------
// Muon pT-smearing with systematic variations
//---------------------------------------------
void nanoAna::correctionlib_muon_smearpT(Particle &muon, const string &_mode) {

  if(_mode=="nom") return;

  float pt  = muon.v.Pt();
  float eta = muon.v.Eta();

  int sign = 0;
  if     (_mode=="systup")   sign = +1;
  else if(_mode=="systdown") sign = -1;
  else return;

  double dpt = 0.01 * pt * sign;
  muon.v.SetPtEtaPhiM(pt + dpt, eta, muon.v.Phi(), muon.v.M());
}

//---------------------------------
// Muon pT scale uncertainty (SF)
//---------------------------------
double nanoAna::correctionlib_muon_ScaleUnc(const Particle &muon, const string &_mode) {
  if     (_mode=="nom")      return 1.0;
  else if(_mode=="systup")   return 1.0 + 0.01;
  else if(_mode=="systdown") return 1.0 - 0.01;
  return 1.0;
}
