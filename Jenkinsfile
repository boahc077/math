#!/usr/bin/env groovy
@Library('StanUtils')
import org.stan.Utils

def runTests(String testPath, boolean jumbo = false) {
    try {
        if (jumbo) {
            sh "./runTests.py -j${env.PARALLEL} ${testPath} --jumbo"
        } else {
            sh "./runTests.py -j${env.PARALLEL} ${testPath}"
        }
    }
        finally { junit 'test/**/*.xml' }
}

def runTestsWin(String testPath, boolean buildLibs = true, boolean jumbo = false) {
    withEnv(['PATH+TBB=./lib/tbb']) {
       bat "echo $PATH"
       if (buildLibs){
           bat "mingw32-make.exe -f make/standalone math-libs"
       }
       try {
           if (jumbo) {
               bat "runTests.py -j${env.PARALLEL} ${testPath} --jumbo"
            } else {
               bat "runTests.py -j${env.PARALLEL} ${testPath}"
            }
       }
       finally { junit 'test/**/*.xml' }
    }
}


def deleteDirWin() {
    bat "attrib -r -s /s /d"
    deleteDir()
}

def skipRemainingStages = false
def skipOpenCL = false

def utils = new org.stan.Utils()

def isBranch(String b) { env.BRANCH_NAME == b }

String alsoNotify() {
    if (isBranch('master') || isBranch('develop')) {
        "stan-buildbot@googlegroups.com"
    } else ""
}
Boolean isPR() { env.CHANGE_URL != null }
String fork() { env.CHANGE_FORK ?: "stan-dev" }
String branchName() { isPR() ? env.CHANGE_BRANCH :env.BRANCH_NAME }
String cmdstan_pr() { params.cmdstan_pr ?: ( env.CHANGE_TARGET == "master" ? "downstream_hotfix" : "downstream_tests" ) }
String stan_pr() { params.stan_pr ?: ( env.CHANGE_TARGET == "master" ? "downstream_hotfix" : "downstream_tests" ) }

pipeline {
    agent none
    parameters {
        string(defaultValue: '', name: 'cmdstan_pr', description: 'PR to test CmdStan upstream against e.g. PR-630')
        string(defaultValue: '', name: 'stan_pr', description: 'PR to test Stan upstream against e.g. PR-630')
        booleanParam(defaultValue: false, name: 'withRowVector', description: 'Run additional distribution tests on RowVectors (takes 5x as long)')
        booleanParam(defaultValue: false, name: 'run_win_tests', description: 'Run full unit tests on Windows.')
    }
    options {
        skipDefaultCheckout()
        preserveStashes(buildCount: 7)
    }
    environment {
        STAN_NUM_THREADS = 4
        CXX = 'clang++-6.0'
        GCC = 'g++'
        MPICXX = 'mpicxx.openmpi'
        N_TESTS = 150
        OPENCL_DEVICE_ID = 0
        OPENCL_DEVICE_ID_CPU = 0
        OPENCL_DEVICE_ID_GPU = 0
        OPENCL_PLATFORM_ID = 1
        OPENCL_PLATFORM_ID_CPU = 0
        OPENCL_PLATFORM_ID_GPU = 0
        PARALLEL = 8
    }
    stages {
// Need Administrator Script approval
//         stage('Kill previous builds') {
//             when {
//                 not { branch 'develop' }
//                 not { branch 'master' }
//             }
//             steps {
//                 script {
//                     utils.killOldBuilds()
//                 }
//             }
//         }
//         stage("Clang-format") {
//             agent {
//                 docker {
//                     image 'stanorg/ci:ubuntu'
//                     label 'linux'
//                 }
//             }
//             steps {
//                 retry(3) { checkout scm }
//                 withCredentials([usernamePassword(credentialsId: 'a630aebc-6861-4e69-b497-fd7f496ec46b',
//                     usernameVariable: 'GIT_USERNAME', passwordVariable: 'GIT_PASSWORD')]) {
//                     sh """#!/bin/bash
//                         set -x
//                         git checkout -b ${branchName()}
//                         clang-format --version
//                         find stan test -name '*.hpp' -o -name '*.cpp' | xargs -n20 -P${PARALLEL} clang-format -i
//                         if [[ `git diff` != "" ]]; then
//                             git config user.email "mc.stanislaw@gmail.com"
//                             git config user.name "Stan Jenkins"
//                             git add stan test
//                             git commit -m "[Jenkins] auto-formatting by `clang-format --version`"
//                             git push https://${GIT_USERNAME}:${GIT_PASSWORD}@github.com/${fork()}/math.git ${branchName()}
//                             echo "Exiting build because clang-format found changes."
//                             echo "Those changes are now found on stan-dev/math under branch ${branchName()}"
//                             echo "Please 'git pull' before continuing to develop."
//                             exit 1
//                         fi"""
//                 }
//             }
//             post {
//                 always { deleteDir() }
//                 failure {
//                     script {
//                         emailext (
//                             subject: "[StanJenkins] Autoformattted: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'",
//                             body: "Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]' " +
//                                 "has been autoformatted and the changes committed " +
//                                 "to your branch, if permissions allowed." +
//                                 "Please pull these changes before continuing." +
//                                 "\n\n" +
//                                 "See https://github.com/stan-dev/stan/wiki/Coding-Style-and-Idioms" +
//                                 " for setting up the autoformatter locally.\n"+
//                             "(Check console output at ${env.BUILD_URL})",
//                             recipientProviders: [[$class: 'RequesterRecipientProvider']],
//                             to: "${env.CHANGE_AUTHOR_EMAIL}"
//                         )
//                     }
//                 }
//             }
//         }
//         stage('Linting & Doc checks') {
//             agent {
//                 docker {
//                     image 'stanorg/ci:ubuntu'
//                     label 'linux'
//                 }
//             }
//             steps {
//                 script {
//                     retry(3) { checkout scm }
//                     sh "git clean -xffd"
//                     stash 'MathSetup'
//                     sh "echo CXX=${CXX} -Werror > make/local"
//                     sh "echo BOOST_PARALLEL_JOBS=${PARALLEL} >> make/local"
//                     parallel(
//                         CppLint: { sh "make cpplint" },
//                         Dependencies: { sh """#!/bin/bash
//                             set -o pipefail
//                             make test-math-dependencies 2>&1 | tee dependencies.log""" } ,
//                         Documentation: { sh "make doxygen" },
//                     )
//                 }
//             }
//             post {
//                 always {
//                     recordIssues enabledForFailure: true, tools:
//                         [cppLint(),
//                          groovyScript(parserId: 'mathDependencies', pattern: '**/dependencies.log')]
//                     deleteDir()
//                 }
//             }
//         }
        stage('Verify changes') {
            agent {
                docker {
                    image 'stanorg/ci:ubuntu'
                    label 'linux'
                }
            }
            steps {
                script {

                    retry(3) { checkout scm }
                    sh 'git clean -xffd'

                    def paths = ['stan', 'make', 'lib', 'test', 'runTests.py', 'runChecks.py', 'makefile', 'Jenkinsfile', '.clang-format'].join(" ")
                    skipRemainingStages = utils.verifyChanges(paths)

                    def openCLPaths = ['stan/math/opencl', 'test/unit/math/opencl'].join(" ")
                    skipOpenCL = utils.verifyChanges(openCLPaths)
                }
            }
        }
//         stage('Headers check') {
//             agent {
//                 docker {
//                     image 'stanorg/ci:ubuntu'
//                     label 'linux'
//                 }
//             }
//             when {
//                 expression {
//                     !skipRemainingStages
//                 }
//             }
//             steps {
//                 unstash 'MathSetup'
//                 sh "echo CXX=${CXX} -Werror > make/local"
//                 sh "make -j${PARALLEL} test-headers"
//             }
//             post { always { deleteDir() } }
//         }
        stage('Full Unit Tests') {
            agent {
                docker {
                    image 'stanorg/ci:ubuntu'
                    label 'linux'
                }
            }
            when {
                expression {
                    !skipRemainingStages
                }
            }
            steps {
                unstash 'MathSetup'
                // Set Stan local compiler flags to use the new TBB interface
                sh """
                    export TBB_INC=\$(pwd)/lib/tbb_2020.3/include
                    export TBB_LIB=\$(pwd)/lib/tbb
                    mkdir -p ~/.config/stan
                    echo TBB_INTERFACE_NEW=true>> ~/.config/stan/make.local
                """
	            sh "echo CXXFLAGS += -fsanitize=address > make/local"
                script {
                    if (isUnix()) {
                        runTests("test/unit", true)
                    } else {
                        runTestsWin("test/unit", true)
                    }
                }
            }
            post { always { retry(3) { deleteDir() } } }
        }
        stage('Always-run tests') {
            when {
                expression {
                    !skipRemainingStages
                }
            }
            failFast true
            parallel {
                stage('MPI tests') {
                    agent {
                        docker {
                            image 'stanorg/ci:ubuntu'
                            label 'linux'
                        }
                    }
                    steps {
                        sh "echo CXX=${MPICXX} >> make/local"
                        sh "echo CXX_TYPE=gcc >> make/local"
                        sh "echo STAN_MPI=true >> make/local"
                        runTests("test/unit/math/prim/functor")
                        runTests("test/unit/math/rev/functor")
                    }
                    post { always { retry(3) { deleteDir() } } }
                }
                stage('OpenCL CPU tests') {
                    agent {
                        docker {
                            image 'stanorg/ci:gpu'
                            label 'gpu'
                            args '--gpus 1'
                        }
                    }
                    when {
                        expression {
                            !skipOpenCL
                        }
                    }
                    steps {
                        script {
                            if (isUnix()) {
                                unstash 'MathSetup'
                                sh "echo CXX=${CXX} -Werror > make/local"
                                sh "echo STAN_OPENCL=true>> make/local"
                                sh "echo OPENCL_PLATFORM_ID=${OPENCL_PLATFORM_ID_CPU}>> make/local"
                                sh "echo OPENCL_DEVICE_ID=${OPENCL_DEVICE_ID_CPU}>> make/local"
                                // skips tests that require specific support in OpenCL
                                sh 'echo "ifdef NO_CPU_OPENCL_INT64_BASE_ATOMIC" >> make/local'
                                sh 'echo "CXXFLAGS += -DSTAN_TEST_SKIP_REQUIRING_OPENCL_INT64_BASE_ATOMIC" >> make/local'
                                sh 'echo "endif" >> make/local'

                                runTests("test/unit/math/opencl", false)
                                runTests("test/unit/multiple_translation_units_test.cpp")
                            } else {
                                unstash 'MathSetup'
                                bat "echo CXX=${CXX} -Werror > make/local"
                                bat "echo STAN_OPENCL=true >> make/local"
                                bat "echo OPENCL_PLATFORM_ID=${OPENCL_PLATFORM_ID_CPU} >> make/local"
                                bat "echo OPENCL_DEVICE_ID=${OPENCL_DEVICE_ID_CPU} >> make/local"
                                bat 'echo LDFLAGS_OPENCL= -L"C:\\Program Files (x86)\\IntelSWTools\\system_studio_2020\\OpenCL\\sdk\\lib\\x64" -lOpenCL >> make/local'
                                bat "mingw32-make.exe -f make/standalone math-libs"
                                runTestsWin("test/unit/math/opencl", false, false)
                                runTestsWin("test/unit/multiple_translation_units_test.cpp", false, false)
                            }
                        }
                    }
                }
                stage('OpenCL GPU tests') {
                    agent {
                        docker {
                            image 'stanorg/ci:gpu'
                            label 'gpu'
                            args '--gpus 1'
                        }
                    }
                    steps {
                        script {
                            if (isUnix()) {
                                unstash 'MathSetup'
                                sh "echo CXX=${CXX} -Werror > make/local"
                                sh "echo STAN_OPENCL=true>> make/local"
                                sh "echo OPENCL_PLATFORM_ID=${OPENCL_PLATFORM_ID_GPU} >> make/local"
                                sh "echo OPENCL_DEVICE_ID=${OPENCL_DEVICE_ID_GPU} >> make/local"
                                runTests("test/unit/math/opencl", false)
                                runTests("test/unit/multiple_translation_units_test.cpp")
                            } else {
                                unstash 'MathSetup'
                                bat "echo CXX=${CXX} -Werror > make/local"
                                bat "echo STAN_OPENCL=true >> make/local"
                                bat "echo OPENCL_PLATFORM_ID=${OPENCL_PLATFORM_ID_GPU} >> make/local"
                                bat "echo OPENCL_DEVICE_ID=${OPENCL_DEVICE_ID_GPU} >> make/local"
                                bat 'echo LDFLAGS_OPENCL= -L"C:\\Program Files (x86)\\IntelSWTools\\system_studio_2020\\OpenCL\\sdk\\lib\\x64" -lOpenCL >> make/local'
                                bat "mingw32-make.exe -f make/standalone math-libs"
                                runTestsWin("test/unit/math/opencl", false, false)
                                runTestsWin("test/unit/multiple_translation_units_test.cpp", false, false)
                            }

                        }
                    }
                }
                stage('Distribution tests') {
                    agent {
                        docker {
                            image 'stanorg/ci:ubuntu'
                            label 'linux'
                        }
                    }
                    steps {
                        unstash 'MathSetup'
                        sh """
                            echo CXX=${CXX} > make/local
                            echo O=0 >> make/local
                            echo N_TESTS=${N_TESTS} >> make/local
                            """
                        script {
                            if (params.withRowVector || isBranch('develop') || isBranch('master')) {
                                sh "echo CXXFLAGS+=-DSTAN_TEST_ROW_VECTORS >> make/local"
                                sh "echo CXXFLAGS+=-DSTAN_PROB_TEST_ALL >> make/local"
                            }
                        }
                        sh "./runTests.py -j${PARALLEL} test/prob > dist.log 2>&1"
                    }
                    post {
                        always {
                            script { zip zipFile: "dist.log.zip", archive: true, glob: 'dist.log' }
                            retry(3) { deleteDir() }
                        }
                        failure {
                            echo "Distribution tests failed. Check out dist.log.zip artifact for test logs."
                        }
                    }
                }
	            stage('Expressions test') {
                    agent any
                    steps {
                        unstash 'MathSetup'
                        script {
                            sh "echo O=0 > make/local"
                            sh "python ./test/code_generator_test.py"
                            sh "python ./test/signature_parser_test.py"
                            sh "python ./test/statement_types_test.py"
                            sh "python ./test/varmat_compatibility_summary_test.py"
                            sh "python ./test/varmat_compatibility_test.py"
                            withEnv(['PATH+TBB=./lib/tbb']) {
                                sh "python ./test/expressions/test_expression_testing_framework.py"
                                try { sh "./runTests.py -j${PARALLEL} test/expressions" }
                                finally { junit 'test/**/*.xml' }
                            }
                            sh "make clean-all"
                            sh "echo STAN_THREADS=true >> make/local"
                            withEnv(['PATH+TBB=./lib/tbb']) {
                                try {
                                    sh "./runTests.py -j${PARALLEL} test/expressions --only-functions reduce_sum map_rect"
				                }
                                finally { junit 'test/**/*.xml' }
                            }
                        }
                    }
                    post { always { deleteDir() } }
                }
                stage('Threading tests') {
                    agent {
                        docker {
                            image 'stanorg/ci:ubuntu'
                            label 'linux'
                        }
                    }
                    steps {
                        script {
                            unstash 'MathSetup'
                            sh "echo CXX=${CXX} -Werror > make/local"
                            sh "echo STAN_THREADS=true >> make/local"
                            sh "export STAN_NUM_THREADS=4"
                            if (isBranch('develop') || isBranch('master')) {
                                runTests("test/unit")
                                sh "find . -name *_test.xml | xargs rm"
                            } else {
                                runTests("test/unit -f thread")
                                sh "find . -name *_test.xml | xargs rm"
                                runTests("test/unit -f map_rect")
                                sh "find . -name *_test.xml | xargs rm"
                                runTests("test/unit -f reduce_sum")
                            }
                        }
                    }
                    post { always { retry(3) { deleteDir() } } }
                }
                stage('Windows Headers & Unit') {
                    agent { label 'windows' }
                    when {
                        allOf {
                            anyOf {
                                branch 'develop'
                                branch 'master'
                                expression { params.run_win_tests }
                            }
                            expression {
                                !skipRemainingStages
                            }
                        }
                    }
                    steps {
                        unstash 'MathSetup'
                        bat "mingw32-make.exe -f make/standalone math-libs"
                        runTestsWin("test/unit", false, false)
                    }
                }
            }
        }
        stage('Upstream tests') {
            agent {
                docker {
                    image 'stanorg/ci:ubuntu'
                    label 'linux'
                }
            }
            when {
                allOf {
                    expression {
                        env.BRANCH_NAME ==~ /PR-\d+/
                    }
                    expression {
                        !skipRemainingStages
                    }
                }
            }
            steps {
                build(job: "Stan/${stan_pr()}",
                        parameters: [string(name: 'math_pr', value: env.BRANCH_NAME),
                                    string(name: 'cmdstan_pr', value: cmdstan_pr())])
            }
        }
        stage('Upload doxygen') {
            agent {
                docker {
                    image 'stanorg/ci:ubuntu'
                    label 'linux'
                }
            }
            when { branch 'develop'}
            steps {
                retry(3) { checkout scm }
                withCredentials([usernamePassword(credentialsId: 'a630aebc-6861-4e69-b497-fd7f496ec46b',
                                                  usernameVariable: 'GIT_USERNAME', passwordVariable: 'GIT_PASSWORD')]) {
                    sh """#!/bin/bash
                        set -x
                        make doxygen
                        git config user.email "mc.stanislaw@gmail.com"
                        git config user.name "Stan Jenkins"
                        git checkout --detach
                        git branch -D gh-pages
                        git push https://${GIT_USERNAME}:${GIT_PASSWORD}@github.com/stan-dev/math.git :gh-pages
                        git checkout --orphan gh-pages
                        git add -f doc
                        git commit -m "auto generated docs from Jenkins"
                        git subtree push --prefix doc/api/html https://${GIT_USERNAME}:${GIT_PASSWORD}@github.com/stan-dev/math.git gh-pages
                        """
                }
            }
            post { always { deleteDir() } }
        }
    }
    // Below lines are commented to avoid spamming emails during migration/debug
    post {
        always {
            node("linux") {
                recordIssues enabledForFailure: false, tool: clang()
            }
        }
        success {
            script {
                utils.updateUpstream(env, 'stan')
                //utils.mailBuildResults("SUCCESSFUL")
            }
        }
        //unstable { script { utils.mailBuildResults("UNSTABLE", alsoNotify()) } }
        //failure { script { utils.mailBuildResults("FAILURE", alsoNotify()) } }
    }
}
