#!/bin/bash
# run.sh — unified runner for the GEARS example
# Usage:
#   ./run.sh                    # default: .tg geometry, event mode, batch
#   ./run.sh --mode step        # step mode (29 columns)
#   ./run.sh --geometry gdml    # use GDML geometry
#   ./run.sh --vis              # enable visualization
#   ./run.sh --parallel 8       # run 8 parallel jobs
#   ./run.sh --help

set -e
cd "$(dirname "$0")"

# find gears executable
GEARS_BIN="../build/gears"
if [[ ! -x "$GEARS_BIN" ]]; then
  echo "Error: gears not found at $GEARS_BIN"
  echo "Build first: cmake -B build -S . && cmake --build build"
  exit 1
fi

# defaults
GEOM="detector.tg"
OUTMODE="event"
VIS=false
NPARALLEL=1
NEVENTS=50000
OUTPUT="output"

# parse arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --geometry)  GEOM="$2.tg"; [[ "$2" == "gdml" ]] && GEOM="detector.gdml"; shift 2 ;;
    --mode)      OUTMODE="$2"; shift 2 ;;
    --vis)       VIS=true; shift ;;
    --parallel)  NPARALLEL="$2"; shift 2 ;;
    --events)    NEVENTS="$2"; shift 2 ;;
    --output)    OUTPUT="$2"; shift 2 ;;
    --help)
      echo "Usage: ./run.sh [options]"
      echo ""
      echo "Options:"
      echo "  --geometry tg|gdml    Geometry format (default: tg)"
      echo "  --mode event|step     Output mode (default: event)"
      echo "  --vis                 Enable visualization"
      echo "  --parallel N          Run N parallel jobs (default: 1)"
      echo "  --events N            Events per job (default: 50000)"
      echo "  --output PREFIX       Output filename prefix (default: output)"
      echo ""
      echo "Examples:"
      echo "  ./run.sh                           # .tg, event mode, 50k events"
      echo "  ./run.sh --mode step --events 10k  # .tg, step mode, 10k events"
      echo "  ./run.sh --geometry gdml --vis     # GDML + visualization"
      echo "  ./run.sh --parallel 8              # 8 parallel jobs, event mode"
      exit 0
      ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
done

# export for macro substitution
export GEOM OUTMODE
export PHYSLIST=FTFP_BERT_EMZ

# generate GDML from .tg if needed
if [[ "$GEOM" == "detector.gdml" && ! -f detector.gdml ]]; then
  echo "Generating GDML from .tg..."
  cat > /tmp/gears_gdml.mac <<'EOF'
/geometry/source detector.tg
/run/initialize
/geometry/export detector.gdml
EOF
  $GEARS_BIN /tmp/gears_gdml.mac
  rm -f /tmp/gears_gdml.mac
  if [[ ! -f detector.gdml ]]; then
    echo "Error: failed to generate detector.gdml"
    exit 1
  fi
  echo "Generated: detector.gdml"
fi

# build the macro
build_macro() {
  local seed1=$1
  local seed2=$2
  local outfile=$3

  echo "# GEARS example macro"
  echo "/control/verbose 0"
  echo "/geometry/source ${GEOM}"
  echo "/sensitive/add crystal"
  echo "/output/mode ${OUTMODE}"
  echo "/output/maxSteps 50000"
  echo "/run/initialize"
  echo "/gps/particle gamma"
  echo "/gps/energy 2.6 MeV"
  echo "/gps/ang/type iso"
  echo "/gps/ang/mintheta 30 deg"
  echo "/gps/ang/maxtheta 150 deg"
  echo "/analysis/setFileName ${outfile}.root"
  echo "/random/setSeeds ${seed1} ${seed2}"

  if [[ "$VIS" == true ]]; then
    echo "/vis/open OGLSQt"
    echo "/vis/drawVolume"
    echo "/vis/scene/add/trajectories"
    echo "/vis/scene/endOfEventAction accumulate 20"
    echo "/vis/viewer/set/autoRefresh false"
    echo "/vis/viewer/zoom 1.5"
  else
    echo "/vis/disable"
    echo "/tracking/verbose 0"
  fi

  echo "/run/verbose 1"
  echo "/run/printProgress 10000"
  echo "/run/beamOn ${NEVENTS}"

  if [[ "$VIS" == true ]]; then
    echo "/vis/viewer/flush"
    echo "/vis/viewer/set/autoRefresh true"
    echo "/vis/viewer/refresh"
  fi
}

# run single job
run_single() {
  local jobid=${1:-0}
  local seed1=$((RANDOM + jobid * 1000))
  local seed2=$((RANDOM + jobid * 500 + 42))
  local outfile="${OUTPUT}"

  if [[ "$NPARALLEL" -gt 1 ]]; then
    outfile="${OUTPUT}_${jobid}"
  fi

  build_macro "$seed1" "$seed2" "$outfile" > /tmp/gears_job_${jobid}.mac
  $GEARS_BIN /tmp/gears_job_${jobid}.mac
  rm -f /tmp/gears_job_${jobid}.mac
}

# main
echo "========================================="
echo " GEARS Example"
echo "========================================="
echo " Geometry  : ${GEOM}"
echo " Mode      : ${OUTMODE}"
echo " Events    : ${NEVENTS}"
echo " Parallel  : ${NPARALLEL}"
echo " Visualize : ${VIS}"
echo " Output    : ${OUTPUT}.root"
echo "========================================="

if [[ "$NPARALLEL" -gt 1 ]]; then
  echo "Running ${NPARALLEL} parallel jobs..."
  for i in $(seq 0 $((NPARALLEL - 1))); do
    run_single $i &
  done
  wait
  echo "All ${NPARALLEL} jobs done."
  echo "Output files: ${OUTPUT}_0.root .. ${OUTPUT}_$((NPARALLEL-1)).root"
  echo "Merge with: hadd ${OUTPUT}.root ${OUTPUT}_*.root"
else
  run_single
  echo "Done: ${OUTPUT}.root"
fi
