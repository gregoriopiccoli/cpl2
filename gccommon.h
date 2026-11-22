// Parti comuni a tutte le varianti del GC: configurazione, statistiche, misura delle pause, pacer.
#pragma once
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

// percentuale di crescita degli oggetti rispetto ai vivi dell'ultima collect che fa scattare la collect
// automatica (come GOGC in Go). Negativa: collect automatiche disattivate.
#ifndef GC_PERCENT
#define GC_PERCENT 100 //100
#endif
// soglia minima di nuovi oggetti prima di una collect automatica (evita collect continue con heap piccoli)
#ifndef GC_MIN_OBJS
#define GC_MIN_OBJS 5000 // 10000
#endif

namespace gc {

struct GCStats {
  uint64_t allocated=0;    // oggetti registrati
  uint64_t freed=0;        // oggetti reclamati dallo sweep
  uint64_t collections=0;  // collect eseguite (automatiche e manuali)
  uint64_t fullCollections=0; // di cui complete (tutto l'heap)
  uint64_t traces=0;       // chiamate a trace(): misura il lavoro di marcatura
  uint64_t totalPauseNs=0; // tempo totale passato dentro le collect
  uint64_t maxPauseNs=0;   // collect più lunga
  size_t peakObjects=0;    // massimo numero di oggetti registrati contemporaneamente
  double avgPauseUs() const {return collections?double(totalPauseNs)/1000.0/double(collections):0.0;}
};

// misura la durata di una collect e la registra nelle statistiche
class PauseTimer {
  GCStats& s;
  std::chrono::steady_clock::time_point t0;
public:
  explicit PauseTimer(GCStats& st):s(st),t0(std::chrono::steady_clock::now()){}
  PauseTimer(const PauseTimer&)=delete;
  PauseTimer& operator=(const PauseTimer&)=delete;
  ~PauseTimer(){
    uint64_t ns=(uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-t0).count();
    s.collections++;
    s.totalPauseNs+=ns;
    if(ns>s.maxPauseNs) s.maxPauseNs=ns;
  }
};

// decide quando far partire la collect automatica: quando gli oggetti creati dall'ultima collect superano
// percent% dei vivi dopo di essa, con un minimo di minObjs. Così il lavoro di ogni collect (proporzionale
// ai vivi) è ripagato da altrettante allocazioni e il costo per allocazione resta costante.
class Pacer {
public:
  int percent=GC_PERCENT;
  size_t minObjs=GC_MIN_OBJS;
  size_t limit=GC_MIN_OBJS; // soglia corrente
  size_t added=0;           // oggetti creati dall'ultima collect
  bool due() const {return percent>=0 && added>=limit;}
  void updateLimit(size_t live){
    size_t p=percent<0?0:(size_t)percent;
    limit=std::max(minObjs,live/100*p+live%100*p/100);
  }
  void afterCollect(size_t live){added=0;updateLimit(live);}
};

} // namespace gc
