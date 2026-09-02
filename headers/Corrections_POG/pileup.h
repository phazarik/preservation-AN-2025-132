#include "correction.h"
#include <memory>
#include <string>
using namespace std;

/*
  Input files:
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2016preVFP-UL-NanoAODv9/2021-09-10/puWeights.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2016postVFP-UL-NanoAODv9/2021-09-10/puWeights.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2017-UL-NanoAODv9/2021-09-10/puWeights.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run2-2018-UL-NanoAODv9/2021-09-10/puWeights.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-22CDSep23-Summer22-NanoAODv12/2024-01-31/puWeights.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-22EFGSep23-Summer22EE-NanoAODv12/2024-01-31/puWeights.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-23CSep23-Summer23-NanoAODv12/2024-01-31/puWeights.json.gz
  /cvmfs/cms-griddata.cern.ch/cat/metadata/LUM/Run3-23DSep23-Summer23BPix-NanoAODv12/2024-01-31/puWeights.json.gz

  Keys used in each campaign/year:
  - 2016: Collisions16_UltraLegacy_goldenJSON (v0)
  - 2017: Collisions17_UltraLegacy_goldenJSON (v0)
  - 2018: Collisions18_UltraLegacy_goldenJSON (v0)
  - 2022-preEE:    Collisions2022_355100_357900_eraBCD_GoldenJson (v0)
  - 2022-postEE:   Collisions2022_359022_362760_eraEFG_GoldenJson (v0)
  - 2023-preBPix:  Collisions2023_366403_369802_eraBC_GoldenJson (v0)
  - 2023-postBPix: Collisions2023_369803_370790_eraD_GoldenJson (v0)

  Input parameters:
  - float  : Pileup_nTrueInt (from NanoAOD branch )
  - string : nominal/up/down  
*/

//----------------------------
// PU correction pointers
//----------------------------
static unique_ptr<correction::CorrectionSet> pu_json;
static const correction::Correction* puCorr;

//------------------------------------------------------------
// Initialize PU corrections
//------------------------------------------------------------
void nanoAna::initPUCorrections() {
  
  string baseDir = filesystem::path(__FILE__).parent_path().string();
  string filePath = baseDir + "/JSONs/LUM/puWeights_" + string(_campaign) + ".json.gz";
  if (FILE *f = fopen(filePath.c_str(), "r")) fclose(f);
  else { cout << "[ERROR] File not found\n"; return; }
  
  // Key inside JSON
  string key;
  if     (_campaign=="2016preVFP_UL")    key="Collisions16_UltraLegacy_goldenJSON";
  else if(_campaign=="2016postVFP_UL")   key="Collisions16_UltraLegacy_goldenJSON";
  else if(_campaign=="2017_UL")          key="Collisions17_UltraLegacy_goldenJSON";
  else if(_campaign=="2018_UL")          key="Collisions18_UltraLegacy_goldenJSON";
  else if(_campaign=="Run3Summer22")     key="Collisions2022_355100_357900_eraBCD_GoldenJson";
  else if(_campaign=="Run3Summer22EE")   key="Collisions2022_359022_362760_eraEFG_GoldenJson";
  else if(_campaign=="Run3Summer23")     key="Collisions2023_366403_369802_eraBC_GoldenJson"; 
  else if(_campaign=="Run3Summer23BPix") key="Collisions2023_369803_370790_eraD_GoldenJson";  
  else { cout << "[ERROR] Unknown campaign: " << _campaign << endl; return; }

  // Load JSON
  try { 
    pu_json = correction::CorrectionSet::from_file(filePath);
    puCorr  = pu_json->at(key).get();
    cout << ">> Loaded corrections: LUM"<<endl;
  }
  catch (const exception &e) {cout << "[ERROR] Failed to load PU JSON: " << e.what() << endl; return;}
}

//----------------------------
// Pileup SF
//----------------------------
double nanoAna::correctionlib_pileupSF(const string &_mode) {
  if(!puCorr) { cout << "[ERROR] PU corrections not initialized!\n"; return 1.0; }
  if(*Pileup_nTrueInt > 99.0) return 1.0;
  
  string mode;
  if      (_mode=="nom")      mode="nominal";
  else if (_mode=="systup")   mode="up";
  else if (_mode=="systdown") mode="down";
  else { cout << "[ERROR] Wrong syst mode: " << _mode << endl; return 1.0; }

  vector<correction::Variable::Type> values;
  values.emplace_back(*Pileup_nTrueInt);
  values.emplace_back(mode);

  return puCorr->evaluate(values);
}
