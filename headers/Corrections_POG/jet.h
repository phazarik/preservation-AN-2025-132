#include "correction.h"
#include "TRandom3.h"
#include <memory>
#include <string>
#include <TLorentzVector.h>
#include <filesystem>
using namespace std;

/*
  Input files:
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2016preVFP-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2016postVFP-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2017-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run2-2018-UL-NanoAODv9/2025-04-11/jet_jerc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-22CDSep23-Summer22-NanoAODv12/2025-09-23/jet_jerc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-22EFGSep23-Summer22EE-NanoAODv12/2025-10-07/jet_jerc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-23CSep23-Summer23-NanoAODv12/2025-10-07/jet_jerc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/JME/Run3-23DSep23-Summer23BPix-NanoAODv12/2025-10-07/jet_jerc.json.gz
  
  Keys used in Run2/Run3:
  1. JEC keys
  - Summer19UL16APV_V7_MC_Total_AK4PFchs (v2)
  - Summer19UL16_V7_MC_Total_AK4PFchs (v2)
  - Summer19UL17_V5_MC_Total_AK4PFchs (v2)
  - Summer19UL18_V5_MC_Total_AK4PFchs (v2)
  - Summer22_22Sep2023_V3_MC_Total_AK4PFPuppi (v3)
  - Summer22EE_22Sep2023_V3_MC_Total_AK4PFPuppi (v3)
  - Summer23Prompt23_V2_MC_Total_AK4PFPuppi (v2)
  - Summer23BPixPrompt23_V3_MC_Total_AK4PFPuppi (v2)

  2. JER keys
  - Summer20UL16APV_JRV3_MC_ScaleFactor_AK4PFchs (v2)
  - Summer20UL16_JRV3_MC_ScaleFactor_AK4PFchs (v2)
  - Summer19UL17_JRV2_MC_ScaleFactor_AK4PFchs (v2)
  - Summer19UL18_JRV2_MC_ScaleFactor_AK4PFchs (v2)
  - Summer22_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi (v3)
  - Summer22EE_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi (v3)
  - Summer23Prompt23_RunCv1234_JRV1_MC_ScaleFactor_AK4PFPuppi (v2)
  - Summer23BPixPrompt23_RunD_JRV1_MC_ScaleFactor_AK4PFPuppi (v2)

  3. pT-resolution keys:
  - Summer20UL16APV_JRV3_MC_PtResolution_AK4PFchs (v2)
  - Summer20UL16_JRV3_MC_PtResolution_AK4PFchs (v2)
  - Summer19UL17_JRV2_MC_PtResolution_AK4PFchs (v2)
  - Summer19UL18_JRV2_MC_PtResolution_AK4PFchs (v2)
  - Summer22_22Sep2023_JRV1_MC_PtResolution_AK4PFPuppi (v3)
  - Summer22EE_22Sep2023_JRV1_MC_PtResolution_AK4PFPuppi (v3)
  - Summer23Prompt23_RunCv1234_JRV1_MC_PtResolution_AK4PFPuppi (v2)
  - Summer23BPixPrompt23_RunD_JRV1_MC_PtResolution_AK4PFPuppi (v2)

  Input parameters:
  1. JEC
  - float: eta [-5.4, 5.4)
  - float: pT [-inf, inf)
  
  2. JER
  - float: eta [-5.191, 5.191)
  - float: [10.0, 4000.0) **Run3 only
  - string: mode [nom, up, down]

  3. pT-resolution
  - float: eta [-5.191, 5.191) in Run3, [-4.7, 4.7) in Run2
  - float: pT [-inf, inf)
  - float: rho [0.0, 90.0) in Run2, [0.0, 60.0) in 2022, [0.0, 52.0) in 2023
  
*/

//----------------------------
// JSON objects
//----------------------------
static unique_ptr<correction::CorrectionSet> jme_json;

// JEC / JER / pt resolution pointers
static const correction::Correction* jecsfjson;
static const correction::Correction* jersfjson;
static const correction::Correction* ptresjson;

//----------------------------
// Initialize JME corrections
//----------------------------
void nanoAna::initJetCorrections() {

  string baseDir = filesystem::path(__FILE__).parent_path().string();
  string filePath = baseDir + "/JSONs/JME/jet_jerc_" + string(_campaign) + ".json.gz";
  if (FILE *f = fopen(filePath.c_str(), "r")) fclose(f);
  else { cout << "[ERROR] File not found\n"; return; }

  try { jme_json = correction::CorrectionSet::from_file(filePath); }
  catch (const exception &e) { cout << "Failed to load JME JSON: " << e.what() << endl; return;}

   if (_campaign=="2016preVFP_UL") {
    jecsfjson  = jme_json->at("Summer19UL16APV_V7_MC_Total_AK4PFchs").get();
    jersfjson  = jme_json->at("Summer20UL16APV_JRV3_MC_ScaleFactor_AK4PFchs").get();
    ptresjson  = jme_json->at("Summer20UL16APV_JRV3_MC_PtResolution_AK4PFchs").get();
  }
  else if (_campaign=="2016postVFP_UL") {
    jecsfjson  = jme_json->at("Summer19UL16_V7_MC_Total_AK4PFchs").get();
    jersfjson  = jme_json->at("Summer20UL16_JRV3_MC_ScaleFactor_AK4PFchs").get();
    ptresjson  = jme_json->at("Summer20UL16_JRV3_MC_PtResolution_AK4PFchs").get();
  }
  else if (_campaign=="2017_UL") {
    jecsfjson  = jme_json->at("Summer19UL17_V5_MC_Total_AK4PFchs").get();
    jersfjson  = jme_json->at("Summer19UL17_JRV2_MC_ScaleFactor_AK4PFchs").get();
    ptresjson  = jme_json->at("Summer19UL17_JRV2_MC_PtResolution_AK4PFchs").get();
  }
  else if (_campaign=="2018_UL") {
    jecsfjson  = jme_json->at("Summer19UL18_V5_MC_Total_AK4PFchs").get();
    jersfjson  = jme_json->at("Summer19UL18_JRV2_MC_ScaleFactor_AK4PFchs").get();
    ptresjson  = jme_json->at("Summer19UL18_JRV2_MC_PtResolution_AK4PFchs").get();
  }
  else if (_campaign=="Run3Summer22") {
    jecsfjson  = jme_json->at("Summer22_22Sep2023_V3_MC_Total_AK4PFPuppi").get();
    jersfjson  = jme_json->at("Summer22_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi").get();
    ptresjson  = jme_json->at("Summer22_22Sep2023_JRV1_MC_PtResolution_AK4PFPuppi").get();
  }
  else if (_campaign=="Run3Summer22EE") {
    jecsfjson  = jme_json->at("Summer22EE_22Sep2023_V3_MC_Total_AK4PFPuppi").get();
    jersfjson  = jme_json->at("Summer22EE_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi").get();
    ptresjson  = jme_json->at("Summer22EE_22Sep2023_JRV1_MC_PtResolution_AK4PFPuppi").get();
  }
  else if (_campaign=="Run3Summer23") {
    jecsfjson  = jme_json->at("Summer23Prompt23_V2_MC_Total_AK4PFPuppi").get();
    jersfjson  = jme_json->at("Summer23Prompt23_RunCv1234_JRV1_MC_ScaleFactor_AK4PFPuppi").get();
    ptresjson  = jme_json->at("Summer23Prompt23_RunCv1234_JRV1_MC_PtResolution_AK4PFPuppi").get();
  }
  else if (_campaign=="Run3Summer23BPix") {
    jecsfjson  = jme_json->at("Summer23BPixPrompt23_V3_MC_Total_AK4PFPuppi").get();
    jersfjson  = jme_json->at("Summer23BPixPrompt23_RunD_JRV1_MC_ScaleFactor_AK4PFPuppi").get();
    ptresjson  = jme_json->at("Summer23BPixPrompt23_RunD_JRV1_MC_PtResolution_AK4PFPuppi").get();
  }
  else {cout<<"JME: wrong campaign. failed to load corrections."<<endl; return;}
  cout << ">> Loaded corrections: JME" << endl;
}

//--------------------------
// Jet Energy Correction SF
//--------------------------
double nanoAna::correctionlib_JEC(const TLorentzVector &jet, const string &_mode) {

  float pt  = jet.Pt();
  float eta = jet.Eta();
  if (pt < 15.) return 1.0;

  vector<correction::Variable::Type> values;
  values.emplace_back(eta);
  values.emplace_back(pt);
  float unc = jecsfjson->evaluate(values);
  
  if      (_mode=="systup")   return 1.0 + unc;
  else if (_mode=="systdown") return 1.0 - unc;
  else                        return 1.0;
}

//-------------------------------------
// Jet Energy Resolution SF / Smearing
//-------------------------------------
double nanoAna::correctionlib_JER(const TLorentzVector &jet, const string &_mode) {

  float pt  = jet.Pt();
  float eta = jet.Eta();
  float phi = jet.Phi();
  float rho = *fixedGridRhoFastjetAll;

  string mode;
  if      (_mode=="nom")      mode="nom";
  else if (_mode=="systup")   mode="up";
  else if (_mode=="systdown") mode="down";
  else { cout<<"JME: Wrong syst mode "<<_mode<<endl; return 1.0; }
  
  //Extract JER
  vector<correction::Variable::Type> jersfvalues; jersfvalues.clear();
  jersfvalues.emplace_back(eta);
  if (_year > 2019) jersfvalues.emplace_back(pt);
  jersfvalues.emplace_back(mode);
  float sjer = 1.0;
  if (fabs(eta) < 5.191) sjer = jersfjson->evaluate(jersfvalues);
  
  //Extract pT-resolution
  vector<correction::Variable::Type> ptresvalues; ptresvalues.clear();
  ptresvalues.emplace_back(eta);
  ptresvalues.emplace_back(pt);
  ptresvalues.emplace_back(rho);
  float sigmajer = 1.0;
  float rhocutoff = 90.0;
  if (_year == 2022) rhocutoff = 60.0;
  if (_year == 2023) rhocutoff = 52.0;
  if (_year < 2019 && rho<rhocutoff && fabs(eta) < 4.7)   sigmajer = ptresjson->evaluate(ptresvalues);
  if (_year > 2019 && rho<rhocutoff && fabs(eta) < 5.191) sigmajer = ptresjson->evaluate(ptresvalues);
  
  // Correction:
  float cjer = 1.0;

  // Gen-matching: find closest gen-jet
  bool  isGenMatched = false;
  float mindR = 1e3;
  float genpt = -1;
  for (int i = 0; i < (int)genJet.size(); i++) {
    float tempdR = jet.DeltaR(genJet.at(i).v);
    if (tempdR < mindR) {
      mindR = tempdR;
      genpt = genJet.at(i).v.Pt();
    }
  }
  if (mindR < 0.2 && abs(pt - genpt) < 3 * pt * sigmajer) isGenMatched = true;
  
  // Hybrid smearing: https://cms-jerc.web.cern.ch/JER/
  if (isGenMatched) cjer = 1.0 + (sjer - 1.0) * ((pt - genpt) / pt);
  else {
    TRandom3 gRandom(0);
    float n = gRandom.Gaus(0.0, sigmajer);
    cjer = 1.0 + n * sqrt(max(sjer * sjer - 1.0f, 0.0f));
  }

  if (cjer < 0.) cjer = 0.0;
  return cjer;
}
