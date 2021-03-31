package com.inaccel.coral;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public final class Library {

	private static final String PATH = "native/lib%s.so";

	public static void load(String libname, Class clazz) {
		String name = String.format(PATH, libname);

		try (InputStream input = clazz.getClassLoader().getResourceAsStream(name);) {
			File library = File.createTempFile("lib", ".so");

			try (OutputStream output = new FileOutputStream(library);) {
				byte[] b = new byte[8192];
				int len;
				while ((len = input.read(b)) > 0) {
					output.write(b, 0, len);
				}

				System.load(library.getAbsolutePath());
			} finally {
				library.delete();
			}
		} catch (Exception e) {
			throw new LinkageError(name, e);
		}
	}

}
