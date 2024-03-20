use std::ffi::CStr;
use std::os::raw::c_char;
const RR_NUM_MAX: usize = 128;

use serde::{Serialize, Deserialize};
//use sonic_rs::{Deserialize, Serialize};
use serde_json::{Result, Value, Error};


#[repr(C)]
pub struct NapCmd_C {
    rr_num: u8,
    bt: *const libc::c_char,
    sbt: *const libc::c_char,
    qtype: *const libc::c_char,
    rr: [*const libc::c_char; RR_NUM_MAX],
}



#[derive(Serialize, Deserialize, Debug)]
struct NapCmd {
    bt: String,
    sbt: String,
    qtype: String,
    rr: Vec<String>,
}

impl NapCmd {
    fn from_raw(raw: &NapCmd_C) -> NapCmd {
        let bt = unsafe { CStr::from_ptr(raw.bt) };
        let sbt = unsafe { CStr::from_ptr(raw.sbt) };
        let qtype = unsafe { CStr::from_ptr(raw.qtype) };
        
        let bt_str = bt.to_string_lossy().into_owned();
        let sbt_str = sbt.to_string_lossy().into_owned();
        let qtype_str = qtype.to_string_lossy().into_owned();
        
        let rr: Vec<String> = raw.rr[..raw.rr_num as usize].iter().map(|&ptr| {
            unsafe { CStr::from_ptr(ptr) }
        }).map(|cstr| {
            cstr.to_string_lossy().into_owned()
        }).collect();
        
        NapCmd {
            bt: bt_str,
            sbt: sbt_str,
            qtype: qtype_str,
            rr: rr,
        }
    }
}


//{"bt":"dns-force", "sbt":"dn_limit", "qtype":"aaaa", "rr":["1.1.1.1", "2.2.2.2" ]  }

#[no_mangle]
pub extern "C" fn test_rust_json_read(string: *const c_char) -> i32 {
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


#[no_mangle]
pub extern "C" fn test_rust_json_write(raw: &NapCmd_C) -> i32 {
    //println!("Try write point: {}, {}", raw.rr_num, raw.num);
    let nap_cmd = NapCmd::from_raw(raw);
    let json_string = serde_json::to_string(&nap_cmd).expect("Failed to convert to JSON string");
    //println!("Try write point: {}", json_string);
    //println!("Try write point: {:?}", nap_cmd);
    return 1;
}
