void nanoAna::initCustomCorrections(){
  cout << ">> Loading custom MC corrections..." << endl;

  // Dynamically get the directory containing this exact header file
  std::string baseDir = std::filesystem::path(__FILE__).parent_path().string();
  TString p = baseDir + "/";

  // Helper to parse global JSONs
  auto parseGlobal = [](TString filepath, map<TString, map<int, GlobalSF>>& sf_map) {
    ifstream f(filepath.Data());
    if (!f.good()) { cout << "[ERROR] Could not open " << filepath << endl; return; }
    json j = json::parse(f);
    for (auto& [camp, channels] : j.items()) {
      for (auto& [chan, values] : channels.items()) {
        GlobalSF gsf;
        gsf.scale = values[0].get<double>();
        if (values.size() > 1) gsf.stat = values[1].get<double>();
        sf_map[camp][stoi(chan)] = gsf;
      }
    }
  };

  // Helper to parse binned JSONs (handles missing stat and separated syst)
  auto parseBinned = [](TString sf_path, TString syst_path,
                        map<TString, map<int, vector<BinnedSF>>>& sf_map) {

    ifstream fsf(sf_path.Data());
    if (!fsf.good()) { cout << "[ERROR] Could not open " << sf_path << endl; return; }
    json jsf = json::parse(fsf);

    json jsyst;
    bool hasSyst = false;
    if (syst_path != "") {
      ifstream fsyst(syst_path.Data());
      if (fsyst.good()) { jsyst = json::parse(fsyst); hasSyst = true; }
      else { cout << "[ERROR] Could not open " << syst_path << endl; return; }
    }

    for (auto& [camp, channels] : jsf.items()) {
      for (auto& [chan, bins] : channels.items()) {
        int ch = stoi(chan);
        for (size_t i = 0; i < bins.size(); i++) {
          BinnedSF binSF;
          binSF.low = bins[i]["low"].get<double>();

          if (bins[i]["high"].is_string() && bins[i]["high"].get<string>() == "inf") {
            binSF.high = numeric_limits<double>::infinity();
          }
          else {
            binSF.high = bins[i]["high"].get<double>();
          }

          // Safely extract scale and stat
          if (bins[i].contains("scale")) {
            binSF.scale = bins[i]["scale"][0].get<double>();
            if (bins[i]["scale"].size() > 1) binSF.stat = bins[i]["scale"][1].get<double>();
          }

          // Attach systematic if available
          if (hasSyst && jsyst.contains(camp) && jsyst[camp].contains(chan)) {
            binSF.syst = jsyst[camp][chan][i]["syst"].get<double>();
          }
	  
          sf_map[camp][ch].push_back(binSF);
        }
      }
    }
  };

  parseGlobal(p + "2LSS_qcdcr_combined_global.json", sf_qcd);
  parseGlobal(p + "2LSS_wjcr_combined_global.json", sf_wj);
  parseGlobal(p + "2LOS_ttxcr_individual_global.json", sf_ttx_global); 
  parseBinned(p + "2LOS_dycr_individual_inBinsOf_search0.json", p + "dy_syst_inBinsOf_search0.json", sf_dy);
  parseBinned(p + "2LOS_ttxcr_individual_inBinsOf_HT.json", "", sf_ttx);
}

double nanoAna::returnGlobalSF(const map<TString, map<int, GlobalSF>>& sf_map, TString systMode) {
  if (sf_map.find(_campaign) == sf_map.end() ||
      sf_map.at(_campaign).find(_channel) == sf_map.at(_campaign).end()){
    return 1.0; 
  }
  GlobalSF corr = sf_map.at(_campaign).at(_channel);
  if (systMode.Contains("up")) return corr.scale + corr.stat;
  if (systMode.Contains("dn") || systMode.Contains("down")) return corr.scale - corr.stat;
  return corr.scale;
}

double nanoAna::returnBinnedSF(const map<TString, map<int, vector<BinnedSF>>>& sf_map,
			       double varValue, TString systMode) {
  
  if (sf_map.find(_campaign) == sf_map.end() ||
      sf_map.at(_campaign).find(_channel) == sf_map.at(_campaign).end()){
    return 1.0; 
  }
  const auto& bins = sf_map.at(_campaign).at(_channel);
  for (const auto& bin : bins) {
    if (varValue >= bin.low && varValue < bin.high) {
      double total_unc = sqrt((bin.stat * bin.stat) + (bin.syst * bin.syst));
      if (systMode.Contains("up")) return bin.scale + total_unc;
      if (systMode.Contains("dn") || systMode.Contains("down")) return bin.scale - total_unc;
      return bin.scale;
    }
  }
  return 1.0; 
}
