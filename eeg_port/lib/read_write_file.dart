import 'dart:async';
import 'dart:io';
import 'package:path_provider/path_provider.dart';


/*
* Authour: Shehjar Sadhu
* Date : 2021
*
* */
/*
* TODO: Put file write ans read studd here as a seperate class instead of in the queatainnair class as it can be resued across classes instead of copying pasting it everywhere.
* */

class read_write_files {
  Future<String> get _localPath async {
    final directory = await getApplicationDocumentsDirectory();

    return directory.path;
  }

  Future<File> get _localFile async {
    final path = await _localPath;
    return File('$path/eeg3.txt');
  }

  Future<int> readCounter() async {
    try {
      final file = await _localFile;

      // Read the file
      String contents = await file.readAsString();

      return int.parse(contents);
    } catch (e) {
      // If encountering an error, return 0
      return 0;
    }
  }

  Future<File> writeCounter(String counter) async {
    final file = await _localFile;
    return file.writeAsString('$counter');
  }
}
