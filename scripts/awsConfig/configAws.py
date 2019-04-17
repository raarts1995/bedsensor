import boto3
import json
import subprocess

'''
A)	Flash ESP32 software
	1)	retrieve MAC address (using esptool.py if necessary)

B)	Setup AWS IoT and DynamoDB:
	1)	create thing (thingName: mac address)
	2)	create keys and certificates
	3)	attach certificates to device

C)	Flash ESP32 SPIFFS
	1)	store created certificates in files
	2)	flash files to spiffs

'''
printResponses = False

awsCredentialsFile = "awsCredentials.txt"
#AWS access keys
awsAccessKey = ""
awsSecretKey = ""
awsRegion = ""

#same for every device
bedsensorThingType = "Bedsensor"
bedsensorPolicyName = "bedsensorpolicy"

#file paths
keyPath = "../../main/www/"
deviceCertFileName = keyPath + "aws_device_cert.pem.crt"
privateKeyFileName = keyPath + "aws_private_key.pem.key"

readMacCmd = "python ${IDF_PATH}/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/ttyUSB0 read_mac"
def getMACStr():
	p = subprocess.Popen(readMacCmd, stdout=subprocess.PIPE, shell=True)
	out, err = p.communicate()
	res = out.split('\n')
	for line in res:
		if line.find("MAC:") != -1:
			try:
				mac = line.split(' ')[1]
				print(mac)
				return mac
			except:
				print("Index out of range")
	print("No MAC address found")
	return ""

def getAWSCredentials():
	global awsAccessKey, awsSecretKey, awsRegion
	file = None
	try:
		file = open(awsCredentialsFile, "r")
		while True:
			line = file.readline()
			if (line == ""):
				break

			print(line)
			if (line.strip().startswith("#")):
				continue

			key_value = line.split("=")
			if (key_value[0].strip() == "accessKey"):
				print("found access key")
				awsAccessKey = key_value[1].strip()
			elif (key_value[0].strip() == "secretKey"):
				print("found secret key")
				awsSecretKey = key_value[1].strip()
			elif (key_value[0].strip() == "region"):
				print("found region")
				awsRegion = key_value[1].strip()

		return (awsAccessKey != "" and awsSecretKey != "" and awsRegion != "")
	except IOError:
		print("File does not exist")
		print("Creating file...")
		file = open(awsCredentialsFile, "w")
		file.write("# Enter the AWS access key\n")
		file.write("accessKey = ...\n")
		file.write("# Enter the AWS secret key\n")
		file.write("secretKey = ...\n")
		file.write("# Enter the AWS region\n")
		file.write("region    = ...\n")
		file.close()
		return False

########################
### Start of program ###
########################

print("Opening credentials file")
if (not getAWSCredentials()):
	exit()
print(awsAccessKey)
print(awsSecretKey)
print(awsRegion)

print("Retrieving MAC address")
thingName = getMACStr()
if (thingName == ""):
	exit()

if printResponses:
	print("    " + thingName)

session = boto3.Session(
	aws_access_key_id = awsAccessKey,
	aws_secret_access_key = awsSecretKey,
	region_name = awsRegion
)

iot = session.client('iot')
dynamodb = session.client('dynamodb')

print("Check if " + thingName + " already exists")
try:
	response = iot.describe_thing(
		thingName = thingName
	)
	print("    Thing " + thingName + " already exists")
	exit()
except iot.exceptions.ResourceNotFoundException:
	print("    Thing " + thingName + " does not exist")

print("Creating thing "+ thingName)
response = iot.create_thing(
	thingName = thingName,
	thingTypeName = bedsensorThingType
)

thingArn = response["thingArn"]
thingId = response["thingId"]

if printResponses:
	print("    thing Name: " + response["thingName"])
	print("    thing ARN:  " + thingArn)
	print("    thing ID:   " + thingId)

print("Creating keys and certificates")
response = iot.create_keys_and_certificate(
	setAsActive = True
)
certificateArn = response["certificateArn"]
certificateId  = response["certificateId"]
certificatePem = response["certificatePem"]
publicKey     = response["keyPair"]["PublicKey"]
privateKey    = response["keyPair"]["PrivateKey"]

if printResponses:
	print("    certificateArn: " + certificateArn)
	print("    certificateId:  " + certificateId)
	print("    certificatePem: " + certificatePem)
	print("    public key:     " + publicKey)
	print("    private key:    " + privateKey)

print("Attaching certificate to thing")
response = iot.attach_thing_principal(
	thingName = thingName,
	principal = certificateArn
)
#print("    " + json.dumps(response))

print("Attaching policy to certificate")
response = iot.attach_policy(
	policyName = bedsensorPolicyName,
	target = certificateArn
)
#print("    " + json.dumps(response))

'''print("Getting thing endpoint")
response = iot.describe_endpoint()
endpoint = response["endpointAddress"]
if printResponses:
	print("    " + endpoint)'''

print("Writing certificate files")
file = open(deviceCertFileName, "w")
file.write(certificatePem)
file.close()
file = open(privateKeyFileName, "w")
file.write(privateKey)
file.close()

