use std::ffi::CStr;
use std::os::raw::c_char;


use serde::{Serialize, Deserialize};
//use sonic_rs::{Deserialize, Serialize};
use serde_json::{Result, Value, Error};



#[derive(Serialize, Deserialize, Debug)]
struct NapCmd {
    bt: String,
    sbt: String,
    qtype: String,
    rr: Vec<String>,
}

//{"bt":"dns-force", "sbt":"dn_limit", "qtype":"aaaa", "rr":["1.1.1.1", "2.2.2.2" ]  }

#[no_mangle]
pub extern "C" fn test_rust_json_deserialize(string: *const c_char) -> i32 {
    //let deserialized: Result<NapCmd, serde_json::Error> = serde_json::from_str(&json_str);
    //let deserialized: Result<NapCmd, serde_json::Error> = serde_json::from_str::<NapCmd>(&json_str);
    //println!("Error: {}", "try deserialized");
    let c_str = unsafe {
        assert!(!string.is_null());
        CStr::from_ptr(string)
    };

    // 将C语言字符串转换为Rust字符串
    let rust_str = c_str.to_str().expect("Invalid UTF-8 string");

    //println!("Error: {}", rust_str);
    let result: Result<NapCmd> = serde_json::from_str(&rust_str);
    //rintln!("Deserialized point: {:?}", deserialized);
    match result {
        Ok(point) => {
            // 解析成功，继续处理点对象
            //println!("Deserialized point: {:?}", point);
            return 0
        },
        Err(error) => {
            // 解析失败，处理错误信息
            println!("Error: {}", error);
            return 1;
        }
    }
}
/* #[no_mangle]
pub extern "C" fn test_simd_json_deserialize(string: *const c_char) -> i32 {
    //let deserialized: Result<NapCmd, serde_json::Error> = serde_json::from_str(&json_str);
    //let deserialized: Result<NapCmd, serde_json::Error> = serde_json::from_str::<NapCmd>(&json_str);
    //println!("Error: {}", "try deserialized");
    let c_str = unsafe {
        assert!(!string.is_null());
        CStr::from_ptr(string)
    };

    // 将C语言字符串转换为Rust字符串
    let rust_str = c_str.to_str().expect("Invalid UTF-8 string");

    //println!("Error: {}", rust_str);
    let result: Result<NapCmd> = sonic_rs::from_str(&rust_str);
    //rintln!("Deserialized point: {:?}", deserialized);
    match result {
        Ok(point) => {
            // 解析成功，继续处理点对象
            //println!("Deserialized point: {:?}", point);
            return 0
        },
        Err(error) => {
            // 解析失败，处理错误信息
            println!("Error: {}", error);
            return 1;
        }
    }
}  */