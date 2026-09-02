#include "correction.h"
#include <memory>
#include <string>
#include <TLorentzVector.h>
#include <filesystem>
using namespace std;

/*
  --------------
  SCALE-FACTORS
  --------------

  Input files:
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2016preVFP-UL-NanoAODv9/2024-07-02/electron.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2016postVFP-UL-NanoAODv9/2024-07-02/electron.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2017-UL-NanoAODv9/2024-07-02/electron.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run2-2018-UL-NanoAODv9/2024-07-02/electron.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22CDSep23-Summer22-NanoAODv12/2025-12-15/electron.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22EFGSep23-Summer22EE-NanoAODv12/2025-12-15/electron.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23CSep23-Summer23-NanoAODv12/2025-12-15/electron.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23DSep23-Summer23BPix-NanoAODv12/2025-12-15/electron.json.gz
  
  Keys used in Run2/Run3:
  - Run2: UL-Electron-ID-SF (v2)
  - Run3: Electron-ID-SF (v5)

  Input parameters:
  - string: era [2016preVFP, 2016postVFP, 2017, 2018, 2022Re-recoBCD, 2022Re-recoE+PromptFG, 2023PromptC, 2023PromptD]
  - string: mode [sf, sfup, sfdown]
  - string: working-point [Loose, Medium, RecoAbove20, RecoBelow20, Tight, Veto, wp80iso, wp80noiso, wp90iso, wp90noiso] (Run-2)
                      or  [Loose, Medium, PromptMVA-Medium, PromptMVA-Tight, Reco20to75, RecoAbove75, RecoBelow20, Tight, Veto, wp80iso, wp80noiso, wp90iso, wp90noiso] (Run-3)
  - float: eta [-inf, inf)
  - float: pt  [10.0, inf)
  - float: phi [-inf, inf) **for 2023 only.

  ------------
  pT-smearing
  ------------
  
  Input files:
  https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2016postVFP_UL/EGM_ScaleUnc.json.gz
  https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2016preVFP_UL/EGM_ScaleUnc.json.gz
  https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2017_UL/EGM_ScaleUnc.json.gz
  https://github.com/cms-egamma/ScaleFactorsJSON/blob/master/2018_UL/EGM_ScaleUnc.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22CDSep23-Summer22-NanoAODv12/latest/electronSS_EtDependent.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-22EFGSep23-Summer22EE-NanoAODv12/latest/electronSS_EtDependent.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23CSep23-Summer23-NanoAODv12/latest/electronSS_EtDependent.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/EGM/Run3-23DSep23-Summer23BPix-NanoAODv12/latest/electronSS_EtDependent.json.gz

  Keys used in Run2/Run3:
  - Run2: UL-EGM_ScaleUnc
  - Run3: NA

  Input parameters (Run2):
  - string: era [2016preVFP, 2016postVFP, 2017, 2018, 2022Re-recoBCD, 2022Re-recoE+PromptFG, 2023PromptC, 2023PromptD]
  - string: mode ["scaleunc", "systup", "systdown"]
  - float: eta 
  - float: seedgain

*/

static unique_ptr<correction::CorrectionSet> egamma_json;
static unique_ptr<correction::CorrectionSet> egammaSS_json;
static const correction::Correction* egammaID;
static const correction::Correction* egammaReco;
static const correction::Correction* egammaScaleUnc;

//------------------------------------------------------------
// Load the JSON file for a given campaign and year
// Must be initialized before Process()
//------------------------------------------------------------
void nanoAna::initEgammaCorrections() {

  const vector<int> validYears = {2016, 2017, 2018, 2022, 2023};
  if (find(validYears.begin(), validYears.end(), _year) == validYears.end()) {
    cout << ">> EGM corrections not available for year " << _year
	 << ", using SF = 1" << endl;
    egammaID = egammaReco = egammaScaleUnc = nullptr;
    egammaSS_json.reset();
    egamma_json.reset();
    return;
  }

  string baseDir = filesystem::path(__FILE__).parent_path().string();
  
  // Reading SF JSON file:
  string filePath_SF = baseDir + "/JSONs/EGM/electron_" + string(_campaign) + ".json.gz";
  if (FILE *f = fopen(filePath_SF.c_str(), "r")) fclose(f);
  else { cout << "[ERROR] File not found\n"; return; }
  try { egamma_json = correction::CorrectionSet::from_file(filePath_SF); }
  catch (const exception &e) { cout << "Failed to load egamma SF JSON: " << e.what() << endl; return; }

  //Reading Smearing JSON file:
  string filePath_SS = baseDir + "/JSONs/EGM/electronSS_" + string(_campaign) + ".json.gz";
  if (FILE *f = fopen(filePath_SS.c_str(), "r")) fclose(f);
  else { cout << "[ERROR] File not found\n"; return; }
  try { egammaSS_json = correction::CorrectionSet::from_file(filePath_SS); }
  catch (const exception &e) { cout << "Failed to load egamma smearing JSON: " << e.what() << endl; return; }
  
  if (_year < 2019) {
    egammaID       = egamma_json->at("UL-Electron-ID-SF").get();
    egammaReco     = egamma_json->at("UL-Electron-ID-SF").get();
    egammaScaleUnc = egammaSS_json->at("UL-EGM_ScaleUnc").get();
  }
  else {
    egammaID       = egamma_json->at("Electron-ID-SF").get();
    egammaReco     = egamma_json->at("Electron-ID-SF").get();
    egammaScaleUnc = nullptr; //Not applying this because of missing variable: supercluster-eta
  }  
  cout << ">> Loaded corrections: EGM" << endl;
}

//-------------------
// Electron Reco SF
//-------------------
double nanoAna::correctionlib_electron_RecoSF(const TLorentzVector &electron, const string &_mode) {
  if (!egammaReco) return 1.0;

  float pt  = electron.Pt();
  float eta = electron.Eta();
  float phi = electron.Phi();
  if(pt <= 10.) return 1.0;

  // Prepare arguments:
  vector<correction::Variable::Type> values;
  
  // Set era
  string era = string(_campaign);
  if     (_campaign=="2016preVFP_UL")    era="2016preVFP";
  else if(_campaign=="2016postVFP_UL")   era="2016postVFP";
  else if(_campaign=="2017_UL")          era="2017";
  else if(_campaign=="2018_UL")          era="2018";
  else if(_campaign=="Run3Summer22")     era="2022Re-recoBCD";
  else if(_campaign=="Run3Summer22EE")   era="2022Re-recoE+PromptFG";
  else if(_campaign=="Run3Summer23")     era="2023PromptC";
  else if(_campaign=="Run3Summer23BPix") era="2023PromptD";
  else { cout<<"[Error]: correctionlib_electron_RecoSF: Could not pick era."<<endl; return 1.0; }
  values.emplace_back(era);
    
  // Set mode
  string mode;
  if(_mode=="nom")           mode="sf";
  else if(_mode=="systup")   mode="sfup";
  else if(_mode=="systdown") mode="sfdown";
  else { cout<<"EGM: Wrong syst mode: "<<_mode<<endl; return 1.0; }
  values.emplace_back(mode);

  // Set working-point
  if(_year < 2019) values.emplace_back(pt < 20.0 ? "RecoBelow20" : "RecoAbove20");
  else {
    if(pt < 20.0)        values.emplace_back("RecoBelow20");
    else if(pt < 75.0)   values.emplace_back("Reco20to75");
    else                 values.emplace_back("RecoAbove75");
  }
  
  // Remaining:
  values.emplace_back(eta);
  values.emplace_back(pt);
  if(_campaign=="Run3Summer23" || _campaign=="Run3Summer23BPix") values.emplace_back(phi);

  return egammaReco->evaluate(values);
}

//-----------------
// Electron ID SF
//-----------------
double nanoAna::correctionlib_electron_IDSF(const TLorentzVector &electron, const string &_mode) {
  if (!egammaID) return 1.0;

  float pt  = electron.Pt();
  float eta = electron.Eta();
  float phi = electron.Phi();
  if(pt <= 10.) return 1.0;

  vector<correction::Variable::Type> values;

  // Set era
  string era = string(_campaign);
  if     (_campaign=="2016preVFP_UL")    era="2016preVFP";
  else if(_campaign=="2016postVFP_UL")   era="2016postVFP";
  else if(_campaign=="2017_UL")          era="2017";
  else if(_campaign=="2018_UL")          era="2018";
  else if(_campaign=="Run3Summer22")     era="2022Re-recoBCD";
  else if(_campaign=="Run3Summer22EE")   era="2022Re-recoE+PromptFG";
  else if(_campaign=="Run3Summer23")     era="2023PromptC";
  else if(_campaign=="Run3Summer23BPix") era="2023PromptD";
  else { cout<<"[Error]: correctionlib_electron_IDSF: Could not pick era."<<endl; return 1.0; }
  values.emplace_back(era);

  // Set mode
  string mode;
  if(_mode=="nom")           mode="sf";
  else if(_mode=="systup")   mode="sfup";
  else if(_mode=="systdown") mode="sfdown";
  else { cout<<"EGM: Wrong syst mode: "<<_mode<<endl; return 1.0; }
  values.emplace_back(mode);

  // Set working-point
  values.emplace_back("Medium");

  // Remaining
  values.emplace_back(eta);
  values.emplace_back(pt);
  if(_campaign=="Run3Summer23" || _campaign=="Run3Summer23BPix") values.emplace_back(phi);

  return egammaID->evaluate(values);
}

//-------------------------------------------------
// Electron pT-smearing with systematic variations
//-------------------------------------------------
void nanoAna::correctionlib_electron_smearpT(Particle &electron, const string &_mode) {
  // Using simple 2% smearing
  int sign = 0;
  if      (_mode == "systup")   sign = +1;
  else if (_mode == "systdown") sign = -1;
  double pt = electron.v.Pt();
  double eta = electron.v.Eta();
  double dpt = 0.02*pt*sign;
  electron.v.SetPtEtaPhiM(pt + dpt, eta, electron.v.Phi(), electron.v.M());
  return;

  /*
  if (!egammaScaleUnc) return 1.0;

  // Note: In Run-2 UL samples, the "EGM_ScaleUnc.json" file contains the keys: "scaledown", "scaleunc", "scaleup" [1]
  //   - scaleunc -> returns the scale uncertainty.
  //   - scaleup/down -> useful when applying the scale itself to samples.
  // Electron pT is already corrected in Run2 UL samples. For systematics evaluation, the scale systematic variations are available only in data [2].
  // Electron pT is not corrected in Run3 samples. Therefore, this function should be called for both data and MC.
  // [1] https://twiki.cern.ch/twiki/bin/view/CMS/EgammaSFJSON
  // [2] https://egammapog.docs.cern.ch/Run2/SaS/#__tabbed_1_1
  
  //if(!egammaScaleUnc) {cout<<"[ERROR] correctionlib_electron_pTscale: JSON not loaded."<<endl; return;} 
  if(_mode=="nom") return;

  float pt   = electron.v.Pt();
  float eta  = electron.v.Eta();
  float E    = electron.v.E();
  int gain   = (int)Electron_seedGain[electron.ind];

  vector<correction::Variable::Type> values;

  //Set era
  string era;
  if     (_campaign=="2016preVFP_UL")    era="2016preVFP";
  else if(_campaign=="2016postVFP_UL")   era="2016postVFP";
  else if(_campaign=="2017_UL")          era="2017";
  else if(_campaign=="2018_UL")          era="2018";
  else if(_campaign=="Run3Summer22")     era="2022Re-recoBCD";
  else if(_campaign=="Run3Summer22EE")   era="2022Re-recoE+PromptFG";
  else if(_campaign=="Run3Summer23")     era="2023PromptC";
  else if(_campaign=="Run3Summer23BPix") era="2023PromptD";
  else { cout<<"[Error]: correctionlib electron ptscale: Could not pick era."<<endl; return;}

  //Set mode
  string mode;
  if      (_mode == "systup")   mode = "scaleup";
  else if (_mode == "systdown") mode = "scaledown";
  
  if(_year<2019){
    if(_data==1){
      values.emplace_back(era);
      values.emplace_back(mode);
      values.emplace_back(eta);
      values.emplace_back(gain);
      double scale = egammaScaleUnc->evaluate(values);
      double dE  = scale * E;
      double dpt = dE / cosh(eta);
      electron.v.SetPtEtaPhiM(pt + dpt, eta, electron.v.Phi(), electron.v.M());
    }
  }
  else {
    // In Run-3, the smearing SFs are given as a function of supercluster-eta.
    // This variable is not available in NanoAODv12.
    // We are going with flat 2% variation.
    int sign = 0;
    if      (_mode == "systup")   sign = +1;
    else if (_mode == "systdown") sign = -1;
    double dpt = 0.02*pt*sign;
    electron.v.SetPtEtaPhiM(pt + dpt, eta, electron.v.Phi(), electron.v.M());
    }*/
}

//---------------------------------
// Electron pT smearing uncertainty
//---------------------------------
double nanoAna::correctionlib_electron_ScaleUnc(const Particle &electron, const string &_mode) {
  // Default:
  if      (_mode == "systup")   return 1.00+0.02;
  else if (_mode == "systdown") return 1.00-0.02;
  return 1.0;

  /*
  if (!egammaScaleUnc) return 1.0;

  // The scale uncertainty arises due to the correction applied in the previous function.
  // This is an additional scale factor (+- uncertainties) applied per electron in MC.

  //if(!egammaScaleUnc) {cout<<"[ERROR] correctionlib_electron_ScaleUnc: JSON not loaded."<<endl; return 1.0;} 
  
  float eta  = electron.v.Eta();
  int gain = (int)Electron_seedGain[electron.ind];

  vector<correction::Variable::Type> values;
  
  //Set era
  string era;
  if     (_campaign=="2016preVFP_UL")    era="2016preVFP";
  else if(_campaign=="2016postVFP_UL")   era="2016postVFP";
  else if(_campaign=="2017_UL")          era="2017";
  else if(_campaign=="2018_UL")          era="2018";
  else if(_campaign=="Run3Summer22")     era="2022Re-recoBCD";
  else if(_campaign=="Run3Summer22EE")   era="2022Re-recoE+PromptFG";
  else if(_campaign=="Run3Summer23")     era="2023PromptC";
  else if(_campaign=="Run3Summer23BPix") era="2023PromptD";
  else { cout<<"[Error]: correctionlib electron ptscale unc: Could not pick era."<<endl; return 1.0;}

  if(_year<2019){
    values.emplace_back(era);
    values.emplace_back("scaleunc");
    values.emplace_back(eta);
    values.emplace_back(gain);
    double unc = egammaScaleUnc->evaluate(values);  
    if     (_mode=="nom")      return 1.0;
    else if(_mode=="systup")   return 1.0 + unc;
    else if(_mode=="systdown") return 1.0 - unc;
  }
  else{
    // In Run-3, the smearing SFs are given as a function of supercluster-eta.
    // This variable is not available in NanoAODv12.
    // We are going with flat 2% variation.
    if      (_mode == "systup")   return 1.00+0.02;
    else if (_mode == "systdown") return 1.00-0.02;
  }
  return 1.0;*/
}

