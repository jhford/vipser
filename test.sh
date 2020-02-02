#!/bin/bash

# Make everything
cmake --build . --target vipser vipser-efence sniffer

fail=0;

test() {
  prog=$1
  shift
  name=$1
  shift
  args=$@

  echo TEST $name $prog $args
  $prog $@ &> .test-output
  e=$?
  if [ "$e" -ne 0 ]; then
    echo TEST $name $prog $args FAILED $e "<>" 0
    echo =======================================
    cat .test-output
  fi

  if [ "$WRITE_EXPECTED" == "1" ]; then
    cp output "expected/$name"
  else
    diff output "expected/$name"
    if [ $? -ne 0 ]; then
      echo TEST $name $prog $args FAILED
      echo =============================
      cat .test-output
      fail=1
    else
      echo "TEST $name $prog $args PASSED"
    fi
  fi
}

test_fail() {
  prog=$1
  shift
  name=$1
  shift
  args=$@

  echo TEST_FAIL $name $prog $args
  $prog $@ &> .test-output
  e=$?

  if [ "$e" -eq 0 ]; then
    echo TEST_FAIL $name $prog $args FAILED $e == 0
    echo ==========================================
    cat .test-output
    fail=1
  else
    echo TEST_FAIL $name $prog $args PASSED
  fi
}

for variant in ./vipser ; do #./vipser-efence; do

  # Empty Pipeline
  test $variant empty-pipeline

  # Formats
  for format in jpeg png bmp gif webp tiff; do
    # GIF and BMP don't work as expected
    test $variant export-$format RESIZE,5,5 EXPORT,$format
    test $variant export-$format-q75 RESIZE,5,5 QUALITY,75 EXPORT,$format
    test $variant export-$format-q25 RESIZE,5,5 QUALITY,25 EXPORT,$format
    test $variant export-$format-q95 RESIZE,5,5 QUALITY,95 EXPORT,$format
    test $variant export-$format-q100 RESIZE,5,5 QUALITY,100 EXPORT,$format
  done

  # Resizing Operations
  test $variant resize-down RESIZE,100,100
  test $variant resize-up RESIZE,1500,1500
  test $variant stretch-down STRETCH,100,100
  test $variant stretch-up STRETCH,1500,1500
  test $variant expand-down EXPAND,100,100 # TODO: seems to stretch not expand
  test $variant expand-up EXPAND,1500,1500 # TODO: seems to stetch not expand

  # Extracting
  test $variant extract-all EXTRACT,0,0,1024,768
  test_fail $variant extract-too-big EXTRACT,0,0,1025,769
  test_fail $variant extract-too-big-offset EXTRACT,1,1,1024,768
  test $variant extract-not-all EXTRACT,25,25,50,50

  # Blur
  test $variant blur-decimal BLUR,0.99
  test $variant blur-int BLUR,10
  test $variant blur-bigdecimal BLUR,2.99

  # Rotating
  test $variant autorot AUTOROT
  test $variant autorot-rotate90 AUTOROT ROTATE,90
  for i in 0 90 180 270 360 ; do
    test $variant rotate$i ROTATE,$i
    test_fail $variant rotate$(( i + 1 )) ROTATE,$(( i + 1 ))
  done

  # Chains
  test $variant chain-resize-extract RESIZE,512,384 EXTRACT,$(( 512 / 4)),$((384 / 4)),$((512 / 2)),$((384 / 2))
  test $variant chain-resize-extract-resize RESIZE,512,384 EXTRACT,$(( 512 / 4)),$((384 / 4)),$((512 / 2)),$((384 / 2)) RESIZE,1024,768
  # TODO: chain-all invalid output
  test $variant chain-all RESIZE,512,384 EXTRACT,0,0,100,100 QUALITY,99 EXPORT,webp BLUR,10 AUTOROT ROTATE,90

  # Too many/few arguments
  test_fail $variant resize-too-many RESIZE,100,100,1
  test_fail $variant resize-too-few RESIZE,100
  test_fail $variant stretch-too-many STRETCH,100,100,1
  test_fail $variant stretch-too-few STRETCH,100
  test_fail $variant expand-too-many EXPAND,100,100,1
  test_fail $variant expand-too-few EXPAND,100
  test_fail $variant extract-too-many EXTRACT,0,0,1024,768,0
  test_fail $variant extract-too-few EXTRACT,0,0,1024
  test_fail $variant blur-too-many BLUR,100,100
  test_fail $variant blur-too-few BLUR
  test_fail $variant quality-too-many QUALITY,100,100
  test_fail $variant quality-too-few QUALITY
  test_fail $variant autorot-too-many AUTOROT,100
  test_fail $variant rotate-too-many ROTATE,100,100
  test_fail $variant rotate-too-few ROTATE

done

exit $fail
