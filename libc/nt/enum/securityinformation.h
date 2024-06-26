#ifndef COSMOPOLITAN_LIBC_NT_ENUM_SECURITYINFORMATION_H_
#define COSMOPOLITAN_LIBC_NT_ENUM_SECURITYINFORMATION_H_

#define kNtOwnerSecurityInformation             0x00000001
#define kNtGroupSecurityInformation             0x00000002
#define kNtDaclSecurityInformation              0x00000004
#define kNtSaclSecurityInformation              0x00000008
#define kNtLabelSecurityInformation             0x00000010
#define kNtAttributeSecurityInformation         0x00000020
#define kNtScopeSecurityInformation             0x00000040
#define kNtProcessTrustLabelSecurityInformation 0x00000080
#define kNtAccessFilterSecurityInformation      0x00000100
#define kNtBackupSecurityInformation            0x00010000
#define kNtProtectedDaclSecurityInformation     0x80000000
#define kNtProtectedSaclSecurityInformation     0x40000000
#define kNtUnprotectedDaclSecurityInformation   0x20000000
#define kNtUnprotectedSaclSecurityInformation   0x10000000

#endif /* COSMOPOLITAN_LIBC_NT_ENUM_SECURITYINFORMATION_H_ */
