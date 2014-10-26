import 'package:anbuild/anbuild.dart';

void main(_, port) {
  runFailureGuard(port, () {
    var result = new TargetResult();
    result.addScanSources(['src']);
    result.addIncludes('c', ['includes']);
    result.addIncludes('c++', ['includes']);
    result.addFlags('c', ['-c']);
    result.addFlags('c++', ['-c']);

    // fetch "ansa" dependency
    var ansaUrl = 'https://github.com/unixpickle/ansa.git';
    fetchGitDependency('ansa', ansaUrl, branch: 'v0.2.1').then((_) {
      return runDependency('dependencies/ansa/build.dart');
    }).then((res) {
      result.addFromTargetResult(res);
      port.send(result.pack());
    });
  });
}
