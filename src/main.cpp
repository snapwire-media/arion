//------------------------------------------------------------------------------
//
// Arion
//
// Extract metadata and create beautiful thumbnails of your images.
//
// ------------
//   main.cpp
// ------------
//
// Copyright (c) 2015 Paul Filitchkin, Snapwire
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//    * Neither the name of the organization nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//------------------------------------------------------------------------------

// Local
#include "models/operation.hpp"
#include "models/resize.hpp"
#include "models/read_meta.hpp"
#include "utils/utils.hpp"
#include "arion.hpp"

// Boost
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

// Stdlib
#include <iostream>
#include <string>

using namespace boost::program_options;
using namespace std;

#define ARION_VERSION "0.3.6"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void showHelp(options_description &desc) {
  cerr << desc << endl;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  try {
    positional_options_description p;
    p.add("input", 1);

    string description = "Arion v";
    description += ARION_VERSION;
    description += "\n\n Arguments";

    options_description desc(description);

    desc.add_options()
        ("help", "Produce this help message")
        ("version", "Print version")
        ("input", value<string>(), "The input operations to execute in JSON");

    variables_map vm;

    store(command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

    notify(vm);

    string inputJson;

    if (vm.count("help")) {
      showHelp(desc);
      return 1;
    }

    if (vm.count("version")) {
      cout << "{\"version\":\"" << ARION_VERSION << "\"}" << endl;
      return 0;
    }

    if (vm.count("input")) {
      inputJson = vm["input"].as<string>();
    } else {
      cout << "You must provide the input operations to execute" << endl << endl;
      showHelp(desc);
      return 1;
    }

    Arion arion;

    if (!arion.setup(inputJson)) {
      cout << arion.getJson();
      exit(-1);
    }

    bool result = arion.run();

    cout << arion.getJson();

    if (result) {
      exit(0);
    } else {
      exit(-1);
    }
  }
  catch (std::exception &e) {
    Utils::exitWithError(e.what());
  }

  return 0;
}
