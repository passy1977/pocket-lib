# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [4.2.2] - 2023/06/10
### Fixed
- Fix utf8 conversion in iOS app
- Fix lock on greate new groupon iOS app

## [4.2.1] - 2023/06/01
### Added
### Changed
 - Reintroduce callback on rest call 
 - Change mode to std::move 
### Fixed
- Fix login whit already data
- Fix iOS matching with pocket
- Device reference not load on user
- Fix logout on timeout session
- Fix password generator in web ui

## [4.2.0] - 2023/05/19
### Added
 - Random value generator
 - Multi user
 - Web gui
 - Add random password generator
### Changed
- Removed Scapix support
- Refactor whole project
- Update id from int32 to uint32
- Changed temporary object method creation
### Fixed
- Fix crash on delete
- Fix death lock
- Fix concurrency data


## [4.1.9] - 2021/02/12
### Fixed
- Add new group on first class level

## [4.1.8] - 2021/09/28
### Added
 - Added new iPhone recognition
### Fixed
- Fix lock host address when user is already logged
- Litle fix
- Vertion fix
### Changed
- Update to scapix 1.0.30
- Update namespace in headers

## [4.1.7] - 2021/02/27
### Fixed
- Fix lock host address when user is already logged
- Litle fix
### Changed
- Update to scapix 1.0.29
- Update to cppcommons 1.2.0

## [4.1.6] - 2021/01/20
### Fixed
- Fix crash on server downn
### Changed
- Check connection on Check end point
- Update to cppcommons 1.2.2
### Added
- Add new device to SystemInfo.m
### Changed
- Update Swift code
- Update cppcommons to 1.2.1
- Updare code

## [4.1.5] - 2021/01/17
### Fixed
- Fix timeout
### Added
- Add new device to SystemInfo.m
### Changed
- Update Swift code
- Update cppcommons to 1.2.1
- Updare code

## [4.1.4] - 2021/01/16
### Fixed
- Fix on some case groupField delete synch
- Fix prepareForReuse on link modify
- Fix delete groupFied
- Fix field visualizatione error when shoed empty field

## [4.1.3] - 2021/01/10
### Fixed
- Fix on connection down somme bad access memory

## [4.1.2] - 2020/12/12
### Fixed
- Fix wrong visualization when timeout goes end
- Update to scapix 1.0.25

## [4.1.1] - 2020/09/25
### Fixed
- Fix Group idServer retrive in synch
- Fix FIeld idServer retrive in synch
- Fix on element delete thread overlap
- Fix filter on new element
- Fix filter on modify element
### Changed
- Update to Scapix 1.0.23
- Timeout from 2 minutes to 4
- Update to Xcode 12 for iOS devices

## [4.1.0] - 2020/07/25
### Fixed
- Fix in iOS export and import icon
- Fix keyboad overlap
- Fix automatic logout
### Changed
- Update to Scapix 1.0.21
- Update to CPPCommons 2.1.0
- Change pods from class to struct
- Update log management

## [4.0.1] - 2020/07/07
### Fixed
- Fix export file name
- Fix recicle cell in group
### Changed
- Update to Scapix 1.0.17

## [4.0.0] - 2020/05/07
### Added
- Group self reference
- Add destination server configuration
- Add mutex in synch
### Changed
- Removed all entries project references
- Modify groups ad fields database table
- Modify database loader
- Rebuild menu
- New Group look
- Firt released with Scapix tecnology d

## [Unreleased - And dropped]

### Fixed
- Share data mechanism
- Fix Synchronizator constructor
### Added
- Group self reference
- RAOProperty
- Add permissions
- Add destination server configuration
- Add multi user account
- Add mutex in synch
### Changed
- QML refactoring
- QML Groups page
- Removed all entries project references
- Modify groups ad fields database table
- Modify database loader
- Rebuild menu
- Switch to QSharedPointer memory management
- Reorganization imports
- New Group look
- QML recursive group
- Project reorganization

## [3.3.1] - 2019/11/08
### Fixed
- Update qtcommon
- Chancged Q_DISABLE_COPY to Q_DISABLE_COPY_MOVE

## [3.3.0] - 2019/11/01
### Added
- Share document on MAC and iOS
- Enhancing scrolling in ListView
### Changed
- Macro from DEBUG to ENABLE_DEBUG

## [3.2.2] - 2019/10/28
### Added
- Improved C++ code
- Improved recognition sw version
### Changed
- Rename same classes

## [3.2.1] - 2019/06/26
### Added
- Add face id login on macos and ios
- Add it translation
- Improved javascript code
- Improved C++ code

### Changed
- Rename same classes
### Fixed
- Fix row alignament on rename element

## [3.2.0] - 2019/05/19
### Added
- Improved javascript code
- Improved C++ code
- Add it translation
- Add FaceId login on macos and ios
- Add row selection in fields list
### Changed
- Move data in default data folder
### Fixed
- Fix goups, entries list scroll on delete
- Fix entries list scroll on modify
- Fix tile logout in debug

## [3.1.0] - 2019/05/14
### Added
- Add copy clipboard
- Add row selection into lists
### Fixed
- Fix entries add action

## [3.0.0] - 2019/05/09
### First release
- No additional informations
