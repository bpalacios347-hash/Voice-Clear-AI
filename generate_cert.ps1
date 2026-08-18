$cert = New-SelfSignedCertificate -Subject "CN=VoiceClearAI Test Certificate" -Type CodeSigningCert -CertStoreLocation "Cert:\LocalMachine\My"
$certPath = "C:\Users\Byron\Desktop\VoiceClearAI\VoiceClearAITestCert.cer"
Export-Certificate -Cert $cert -FilePath $certPath
Import-Certificate -FilePath $certPath -CertStoreLocation "Cert:\LocalMachine\Root"
Import-Certificate -FilePath $certPath -CertStoreLocation "Cert:\LocalMachine\TrustedPublisher"
Write-Output $cert.Thumbprint
