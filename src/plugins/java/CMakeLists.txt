cmake_minimum_required(VERSION 3.0.2)

project(plugin-java)

find_package(Qt5 COMPONENTS Xml REQUIRED)

set(CXX_CPP
    maven/mavengenerator.cpp
    maven/mavenbuild.cpp
    maven/builder/mainframe/mavenbuildergenerator.cpp
    maven/builder/parser/mavenparser.cpp
    maven/project/mavenasynparse.cpp
    maven/project/mavenprojectgenerator.cpp
    maven/project/properties/mavenconfigpropertywidget.cpp
    maven/project/properties/mavenconfigutil.cpp
    maven/option/mavenwidget.cpp
    maven/option/mavenoptionwidget.cpp
    maven/option/optionmavengenerator.cpp
    gradle/gradlegenerator.cpp
    gradle/gradlebuild.cpp
    gradle/builder/mainframe/gradlebuildergenerator.cpp
    gradle/builder/parser/gradleparser.cpp
    gradle/project/gradleasynparse.cpp
    gradle/project/gradleprojectgenerator.cpp
    gradle/project/properties/gradleconfigpropertywidget.cpp
    gradle/project/properties/gradleconfigutil.cpp
    gradle/option/gradlewidget.cpp
    gradle/option/gradleoptionwidget.cpp
    gradle/option/optiongradlegenerator.cpp
    java/javadebug.cpp
    java/option/jdkwidget.cpp
    java/option/javaoptionwidget.cpp
    java/option/optionjavagenerator.cpp
    javaplugin.cpp
    javaplugin.json
    javautil.cpp
    )

set(CXX_H
    maven/mavengenerator.h
    maven/mavenbuild.h
    maven/builder/mainframe/mavenbuildergenerator.h
    maven/builder/parser/mavenparser.h
    maven/project/mavenasynparse.h
    maven/project/mavenprojectgenerator.h
    maven/project/properties/mavenconfigpropertywidget.h
    maven/project/properties/mavenconfigutil.h
    maven/option/mavenwidget.h
    maven/option/mavenoptionwidget.h
    maven/option/optionmavengenerator.h
    gradle/gradlegenerator.h
    gradle/gradlebuild.h
    gradle/builder/mainframe/gradlebuildergenerator.h
    gradle/builder/parser/gradleparser.h
    gradle/project/gradleasynparse.h
    gradle/project/gradleprojectgenerator.h
    gradle/project/properties/gradleconfigpropertywidget.h
    gradle/project/properties/gradleconfigutil.h
    gradle/option/gradlewidget.h
    gradle/option/gradleoptionwidget.h
    gradle/option/optiongradlegenerator.h
    java/javadebug.h
    java/option/jdkwidget.h
    java/option/javaoptionwidget.h
    java/option/optionjavagenerator.h
    javaplugin.h
    javautil.h
    )

set(QT_THEME)

set(CXX_QRC
    gradle/project/gradle.qrc
    maven/project/maven.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    ${QT_THEME}
    )

target_link_libraries(${PROJECT_NAME}
    framework
    base
    services
    common
    Qt5::Xml
    ${QtUseModules}
    ${PkgUserModules}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

