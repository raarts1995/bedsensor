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

bedsensorPolicyName = "bedsensorpolicy"

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

thingName = raw_input("Enter the MAC address: ").lower()

if printResponses:
	print("    " + thingName)

session = boto3.Session(
	aws_access_key_id = awsAccessKey,
	aws_secret_access_key = awsSecretKey,
	region_name = awsRegion
)

iot = session.client('iot')

print("Check if " + thingName + " exists")
try:
	response = iot.describe_thing(
		thingName = thingName
	)
	print("    Device " + thingName + " exists")
except iot.exceptions.ResourceNotFoundException:
	print("    Device " + thingName + " does not exist")
	exit()

print("Getting attached certificate")
try:
	response = iot.list_thing_principals(
		thingName = thingName
	)
	certificateArn = response["principals"][0]
	certificateId = certificateArn.split("/")[1]
	print("    Certificate Arn: " + certificateArn)
	print("    Certificate ID:  " + certificateId)

	print("Deactivating certificate")
	response = iot.update_certificate(
		certificateId = certificateId,
		newStatus = "INACTIVE"
	)
	#print("    " + json.dumps(response))

	print("Detaching device")
	response = iot.detach_thing_principal(
		thingName = thingName,
		principal = certificateArn
	)
	#print("    " + json.dumps(response))

	print("Detaching policy")
	response = iot.detach_policy(
		policyName = bedsensorPolicyName,
		target = certificateArn
	)
	#print("    " + json.dumps(response))

	print("Deleting certificate")
	response = iot.delete_certificate(
		certificateId = certificateId
	)
	#print("    " + json.dumps(response))
except IndexError:
	print("No certificate attached")

print("Deleting device " + thingName)
response = iot.delete_thing(
	thingName = thingName
)
#print("    " + json.dumps(response))
