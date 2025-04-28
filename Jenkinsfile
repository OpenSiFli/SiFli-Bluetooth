pipeline {
    agent {
        node {
            label 'Bluetooth'
            }
        }
    stages {
        stage('Print CI Info') {
            steps {
                echo"[NODE_NAME]:${env.NODE_NAME}\n[GERRIT_PROJECT]:${env.GERRIT_PROJECT}\n[GERRIT_BRANCH]:${env.GERRIT_BRANCH}\n[GERRIT_CHANGE_OWNER_EMAIL]:${env.GERRIT_CHANGE_OWNER_EMAIL}\n[GERRIT_CHANGE_SUBJECT]:${env.GERRIT_CHANGE_SUBJECT}\n[GIT_COMMIT]:${env.GIT_COMMIT}\n[GERRIT_PATCHSET_REVISION]:${env.GERRIT_PATCHSET_REVISION}\n[GERRIT_REFSPEC]:${env.GERRIT_REFSPEC}\n" 
                //echo"[WORKSPACE]:${env.WORKSPACE}\n [JENKINS_HOME]:${env.JENKINS_HOME}\n"
            }
        }
        stage('Checkout Code') {
            steps {
                script {
                    dir('../bluetooth_ci') {
                        //echo"[WORKSPACE]:${env.WORKSPACE}\n [JENKINS_HOME]:${env.JENKINS_HOME}\n ${env.GERRIT_REFSPEC}\n ${env.GERRIT_PATCHSET_REVISION}"
                        //sh 'pwd'
                        sh "git reset --hard; git clean -fdx"
                        sh "git submodule foreach --recursive 'git reset --hard; git clean -fdx'"
                        checkout([$class: 'GitSCM',
                            branches: [[name: 'main']],
                            userRemoteConfigs: [[
                                url: 'ssh://jenkins@10.21.10.179:29418/SiFli-SDK',
                                credentialsId: '1'
                            ]],
                            extensions: [
                                [$class: 'CloneOption', depth: 0, shallow: false, timeout: 60],
                                //[$class: 'CloneOption', depth: 30, shallow: true, timeout: 60],
                                [$class: 'SubmoduleOption',
                                    disableSubmodules: false,
                                    parentCredentials: true,
                                    recursiveSubmodules: true,
                                    trackingSubmodules: false,
                                    additionalArguments: '--force',
                                    timeout: 60
                                ]
                            ]
                        ])
                        dir('middleware/bluetooth') {
                            checkout([$class: 'GitSCM',
                                branches: [[name: 'main']],
                                userRemoteConfigs: [[
                                    url: 'ssh://jenkins@10.21.10.179:29418/SiFli-Bluetooth',
                                    credentialsId: '1'
                                ]]
                            ])
                            sshagent(['1']) {
                                sh "git fetch --no-tags --force --progress -- ssh://jenkins@10.21.10.179:29418/SiFli-Bluetooth  ${env.GERRIT_REFSPEC}"
                                sh "git checkout -f ${env.GERRIT_PATCHSET_REVISION}"
                            }
                        }
                        echo 'start build'
                    }
                }
            }
        }
        stage('Parallel Stage') {
            //failFast true
            parallel {
                stage('lb523_central_and_peripheral') {
                    steps {
                        dir('../bluetooth_ci') {
                            bat """
                                tools\\autotest\\build.bat example\\ble\\central_and_peripheral\\project\\hcpu --board eh-lb523_hcpu
                            """
                        }
                    }
                }
                stage('lb563_central_and_peripheral') {
                    steps {
                        dir('../bluetooth_ci') {
                            bat """
                                tools\\autotest\\build.bat example\\ble\\central_and_peripheral\\project\\hcpu --board eh-lb563_hcpu
                            """
                        }
                    }
                }
                stage('lb523_peripheral_with_ota') {
                    steps {
                        dir('../bluetooth_ci') {
                            bat """
                                tools\\autotest\\build.bat example\\ble\\peripheral_with_ota\\project --board eh-lb523_hcpu
                            """
                        }
                    }
                }
                stage('lb523_bt_test_example') {
                    steps {
                        dir('../bluetooth_ci') {
                            bat """
                                tools\\autotest\\build.bat example\\bt\\test_example\\project --board eh-lb523_hcpu
                            """
                        }
                    }
                }
                stage('lb563_bt_test_example') {
                    steps {
                        dir('../bluetooth_ci') {
                            bat """
                                tools\\autotest\\build.bat example\\bt\\test_example\\project --board eh-lb563_hcpu
                            """
                        }
                    }
                }
            }
        }

        stage('Archive files') {
            steps {
                dir('../bluetooth_ci') {
                    archiveArtifacts artifacts: 'middleware\\bluetooth\\lib\\lib_bt_rvds.lib,middleware\\bluetooth\\lib\\lib_bt_msvc.lib,middleware\\bluetooth\\lib\\lib_bt_gcc.a', fingerprint: true, onlyIfSuccessful: true
                    echo "Archive files success"
                }
            }
        }
    }
}

