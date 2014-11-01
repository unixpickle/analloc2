import 'package:anbuild/anbuild.dart';

void main(args, port) {
  runBuildMain(args, port, () {
    var result = new TargetResult();
    result.addScanSources(['src']);
    result.addIncludes('c', ['includes']);
    result.addIncludes('c++', ['includes']);
    result.addFlags('c', ['-c']);
    result.addFlags('c++', ['-c']);

    // fetch "ansa" dependency
    var ansaUrl = 'https://github.com/unixpickle/ansa.git';
    fetchGitDependency('ansa', ansaUrl, branch: 'master').then((_) {
      return runDependency('ansa/build.dart');
    }).then((res) {
      result.addFromTargetResult(res);
      port.send(result.pack());
    });
  });
}
