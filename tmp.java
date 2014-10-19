ByteArrayOutputStream baos = new ByteArrayOutputStream();
DataOutputStream daos = new DataOutputStream(baos);
daos.writeLong(value1);
daos.writeInt(value2);
daos.writeBoolean(value3);
daos.writeUTF(value4);
byte[] result = daos.toByteArray();
